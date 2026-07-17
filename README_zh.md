[English](README.md)

# libreflect

    libreflect是一个使用模板、宏实现的为C++11提供编译时静态反射和运行时动态反射的反射库。<br>
    本库依赖于[libtplmp](https://github.com/xueyufengling/libtplmp/tree/main)和[libppmp](https://github.com/xueyufengling/libppmp/tree/main)，由于预处理器元编程依赖大量预定义的宏，如果宏的数量太多，将会显著地降低编译速度。为了提高编译速度，在安装libppmp时需要根据项目情况调整自动生成宏的参数。<br>

# 许可

    libreflect以LGPL-3.0 Linking Exception许可进行分发，其在原LGPL-3.0许可的基础上移除了您在静态链接时必须提供最小对应源代码的义务。简而言之，只要您不修改本库的源代码，不论您是静态链接还是动态链接，都无需承担开源义务。<br>