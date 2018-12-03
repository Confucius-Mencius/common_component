%module libc_api_swig_module1

// %{ }%中的文字会原封不动地放入生成代码中
%{
#include "c_api_module1.h"
%}

// 需要导出给脚本语言的头文件（各种声明）
%include "c_api_module1.h"

// 加载脚本文件
%luacode {
    --dofile('main.lua') -- 这里放一段lua代码，在加载的时候执行
}
