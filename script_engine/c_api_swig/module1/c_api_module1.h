#ifndef SCRIPT_ENGINE_C_API_SWIG_C_API_MODULE1_H_
#define SCRIPT_ENGINE_C_API_SWIG_C_API_MODULE1_H_

// 全局变量
extern double g_foo;

// const常量
const int MAX_X = 1;

// 枚举
enum SomeType
{
    TYPE_MIN = 0,
    TYPE_1 = TYPE_MIN,
    TYPE_2,
    TYPE_MAX,
};

// c函数
int SomeFunc(int x, int y);

#endif // SCRIPT_ENGINE_C_API_SWIG_C_API_MODULE1_H_
