使用方法：
1，将module1编译成libmodule1.so，将module2编译成libmodule2.so
2，在lua中require('libmodule1')、require('libmodule2')使用

使用建议：
1，swig对namespace的支持与c++语言不一致，生成的lua binding代码中没有namespace了，这一点与toluapp不一样，所以在用swig时禁止添加namespace
2，类型用原生类型名，如int、unsigned int，不要用i32等，以免被swig认为是自定义类型
3，如果生成的so名为libc_api_swig.so，那么.i文件中指定的module名必须为libc_api_swig，lua中require的包名必须为libc_api_swig
4，在lua中调用required加载so时会默认生成一层table，正好可以作为模块的划分
5，各个so内的全局变量、常量、枚举、函数、类、结构体正常命名即可，不需要加前缀
