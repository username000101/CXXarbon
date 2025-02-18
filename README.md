The entire library fits into a single header file. It provides functions for:
Opening the library, closing (automatically), and getting symbols from the library.

Carbon is a programming language developed by Google, it is currently in its early stages, but it is already possible to write in it. 
Current versions of the compiler can already compile simple constructs, and some parts of the standard library are also implemented, but due to specific mangling, files compiled via carbon and linked via ld cannot be run, since the following code:
```carbon
package Simple;

fn main() -> i32 {
    return 0;
}
```
Will contain only the symbol _Cmain.simple, but there are ways to run Carbon code, for example, via dynamic libraries. 
The C/C++ and Carbon ABIs seem to be compatible, so we can simply build a dynamic library without a specific entry point and simply load the desired symbol in the C++ code. 
This is exactly what the library does, providing a slightly friendlier interface than the system apis (I hope)

So, you can download the latest version of the Carbon compiler [here](https://github.com/carbon-language/carbon-lang/releases)

To build your code, you can use the command:
```bash
carbon compile path/to/your/main.carbon
```

After that, in the directory with the file there will be main.o, which you need to link with ld:
(Note: if you are using Core.Print or similar, you need to add the -lc flag when linking, since these functions reference functions from libc)

```bash
ld -o libname.so -shared path/to/your/main.o
```

After linking, just load this library and use symbols from it
