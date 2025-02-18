#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>

#if defined(_WIN32)
#include <libloaderapi.h>
#elif defined(__unix__)
#include <dlfcn.h>
#else
#error "Unknown OS"
#endif

#ifndef NDEBUG
#include <iostream>
#endif

namespace cxxarbon {
    class CarbonLibrary {
    public:
        enum State { OK, ERR, NONE };

        using ErrorDetail = std::pair<State, const char*>;

        CarbonLibrary(const std::string& library_path) {
            if (!std::filesystem::exists(library_path)) {
                this->error_ = {ERR, "File not found"};
                throw std::runtime_error("CarbonLibrary: Failed to load library '" + library_path + "': file not found");
            }

#ifndef NDEBUG
            std::cout << __PRETTY_FUNCTION__ << ": Called constructor with library_path=" << library_path << std::endl;
#endif

#if defined(_WIN32)
            this->library_ = LoadLibrary(library_path.c_str());
            if (!this->library_) {
                auto last_error = GetLastError();
                this->error_ = {ERR, last_error};
                throw std::runtime_error("CarbonLibrary: Failed to load library '" + library_path + "': " + last_error);
            }
#elif defined(__unix__)
            this->library_ = dlopen(library_path.c_str(), RTLD_LAZY);
            if (!this->library_) {
                auto last_error = dlerror();
                this->error_ = {ERR, last_error};
                throw std::runtime_error("CarbonLibrary: Failed to load library '" + library_path + "': " + last_error);
            }
#endif
        }

        ~CarbonLibrary() {
            if (this->library_)
#if defined(_WIN32)
                FreeLibrary(this->library_);
#elif defined(__unix__)
                dlclose(this->library_);
#endif

#ifndef NDEBUG
            std::cout << __PRETTY_FUNCTION__ << ": Called destructor" << std::endl;
#endif
        }

        inline bool is_open() { if (this->library_) return true; else return false; }

        template <typename Signature> inline Signature get_symbol(const std::string& package, const std::string& symbol) {
            if (!this->is_open()) {
                this->error_ = {ERR, "The library is closed"};
                return nullptr;
            }

            std::string full_symbol_name = "_C" + symbol + "." + package;
#if defined(_WIN32)
            Signature symbol_ptr = (Signature)GetProcAddress(this->library_, full_symbol_name.c_str());
            if (!symbol_ptr) {
                this->error_ = {ERR, GetLastError()};
                return nullptr;
            }
#elif defined(__unix__)
            Signature symbol_ptr = (Signature)dlsym(this->library_, full_symbol_name.c_str());
            if (!symbol_ptr) {
                this->error_ = {ERR, dlerror()};
                return nullptr;
            }
#endif
            return symbol_ptr;
        }

        inline ErrorDetail get_last_error() {
            ErrorDetail buf = this->error_;
            this->error_.first = NONE;
            this->error_.second = nullptr;
            return buf;
        }

    private:
#if defined(_WIN32)
        HMODULE library_;
#elif defined(__unix__)
        void* library_;
#endif
        ErrorDetail error_;
    };
}
