使用方法：
1，C/C++提供给lua的函数写在c_api_xxx.h和c_api_xxx.cpp中，其中xxx是模块名
2，如果需要的话，在c_api.pkg中可以嵌入lua代码，在加载的时候会执行
3，执行脚本生成binging代码文件c_api_toluapp.h和c_api_toluapp.cpp
4，将所有的模块编译成一个so
5，在程序中调用tolua_c_api_toluapp_open加载这个so，注意编译时要链接libtolua++.so
5，在lua脚本中通过“模块名.函数名”使用这些c++ api

使用建议：
1，建议在c++中用namespace区分不同的模块，这样用toluapp生成的lua binding api也就是按“模块”划分的了，方便识别和使用
2，namespace命名约定：c_模块名
3，namespace内的全局变量、常量、枚举、函数、类、结构体正常命名即可，不需要加前缀
