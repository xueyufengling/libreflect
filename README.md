[中文](README_zh.md)

# libreflect

    libreflect is a reflection library implemented using templates and macros, providing C++11 with static reflection at compile-time and dynamic reflection at runtime.<br>
    This library depends on [libtplmp](https://github.com/xueyufengling/libtplmp/tree/main) and [libppmp](https://github.com/xueyufengling/libppmp/tree/main). Since preprocessor metaprogramming relies on a large number of predefined macros, an excessive number of macros will significantly slow down compilation. To improve compilation speed, the parameters for automatically generating macros need to be adjusted according to the project's requirements when installing libppmp.<br>

# License

    libreflect is distributed under the LGPL-3.0 with Linking Exception. This license removes the obligation to provide Minimal Corresponding Source when statically linking, which is otherwise required by the original LGPL-3.0 license. In short, as long as you do not modify the source code of this library, you are not required to open-source your code, regardless of whether you choose static or dynamic linking.<br>