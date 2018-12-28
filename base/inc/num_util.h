/**
 * @file num_util.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_NUM_UTIL_H_
#define BASE_INC_NUM_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_NumUtil 数字操作接口、宏
 * @{
 */

#include <math.h>
#include "data_types.h"
#include "float_compare.h"

/**
 1，如果要乘以或除以2^n，用位移运算速度快。a >> n; a << n;
 2，将乘法转换成加法减少时间：log(a*b) = log(a) + log(b) (一定吗？调用log的开销如何？)
 3，将乘法转换成除法防止溢出：a/(b*c)=a/b/c。类似的，将加法转换成减法。
 4，将乘法转换成移位提高效率，如：
 1000 = 1024 - 16 - 8 = 2^10 - 2^4 - 2^3，则Y = 1000*A = A<<10 - A<<4 - A<<3
 5，除法是最耗CPU的，能够转化为等效的乘法会大大提高效率。如果可以，编译器会将除法优化为乘法运算，优化规则：
 除数扩大X倍得到一个数近似等于2^N，这样除法就被转换为“被除数乘以X，再右移N位”了，如：
 A/B = AX/BX = AX >> N
 */

/**
 * @brief MIN
 * @hideinitializer
 */
#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif // MIN

/**
 * @brief MAX
 * @hideinitializer
 */
#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif // MAX

/**
 * @brief ABS
 * @hideinitializer
 */
#ifndef ABS
#define ABS(x) (((x) >= 0) ? (x) : -(x))
#endif // ABS

/**
 * @brief 计算数组元素个数
 * @param array_name 数组名，不能是指针，否则后果不可预知
 * @hideinitializer
 */
#ifndef COUNT_OF
#define COUNT_OF(array_name) (int) (sizeof(array_name) / sizeof(array_name[0]))
#endif

/**
 * @brief 判断是否为非法的index，取值范围[min, max)
 * @hideinitializer
 */
#if __cplusplus >= 201103L
#define INVALID_IDX(idx, min, max) ((idx) < (decltype(idx))(min) || (idx) >= (decltype(idx))(max))
#else
#define INVALID_IDX(idx, min, max) ((idx) < (typeof(idx))(min) || (idx) >= (typeof(idx))(max))
#endif // __cplusplus >= 201103L

/**
 * @brief 交换两个整数的值，T必须是整数类型，否则后果不可预知
 */
template<typename T>
void Swap(T& a, T& b)
{
    b ^= a;
    a ^= b;
    b ^= a;
}

/**
 * @brief 浮点数相等判断，Type为f32或f64
 * @param [in] v1
 * @param [in] v2
 * @return "v1==v2"返回true，否则返回false
 */
template<typename Type>
bool FloatEQ(Type v1, Type v2)
{
    const google_test::FloatingPoint<Type> lhs(v1), rhs(v2);
    return lhs.AlmostEquals(rhs);
}

/**
 * @brief 浮点数大于判断，Type为f32或f64
 * @param [in] v1
 * @param [in] v2
 * @return "v1>v2"返回true，否则返回false
 */
template<typename Type>
bool FloatGT(Type v1, Type v2)
{
    return (v1 > v2) ? true : false;
}

/**
 * @brief 浮点数小于判断，Type为f32或f64
 * @param [in] v1
 * @param [in] v2
 * @return "v1<v2"返回true，否则返回false
 */
template<typename Type>
bool FloatLT(Type v1, Type v2)
{
    return (v1 < v2) ? true : false;
}

/**
 * @brief 防止溢出的加1。
 * @param [in, out] x
 * @hideinitializer
 */
#ifndef INCREASE
#define INCREASE(x) (x = ((x) + 1 > (x)) ? (x) + 1 : (x))
#endif

/**
 * @brief 防止溢出的减1。
 * @param [in, out] x
 * @hideinitializer
 */
#ifndef DECREASE
#define DECREASE(x) (x = ((x) - 1 < (x)) ? (x) - 1 : (x))
#endif

//#define offsetof(s, m) (size_t) &(((s*) 0)->m)

/**
 * @brief 两个整数相除，如果能整除则取准确的商值，否则向上取整
 * @param [in] dividend 被除数
 * @param [in] divisor 除数
 * @return 如果能整除则取准确的商值，否则向上取整
 * @note 尽可能把宏定义改为模板或者内联函数，原因：（1）宏不好调试（2）宏定义容易出漏洞，不安全（3）多行时逻辑容易出错
 */
template<typename T>
T Ceil(T dividend, T divisor)
{
    const T v = dividend / divisor;

    if (0 == (dividend % divisor))
    {
        return v; // 能够整除
    }

    T t = v;
    INCREASE(t);

    return ((v > 0) ? t : v);
}

/**
 * @brief 两个整数相除，取商的整数部分，即向下取整
 * @param [in] dividend 被除数
 * @param [in] divisor 除数
 * @return 商的整数部分
 */
template<typename T>
T Floor(T dividend, T divisor)
{
    const T v = dividend / divisor;

    if (0 == (dividend % divisor))
    {
        return v; // 能够整除
    }

    T t = v;
    DECREASE(t);

    return ((v > 0) ? v : t);
}

/**
 * @brief 浮点数四舍五入，Type为f32或f64
 * @param [in] v
 * @param [in] precision 需要保留的小数位数
 * @return 四舍五入后的结果
 */
template<typename T>
T Round(T v, int precision = 2)
{
    f64 a = v * pow(10, precision);
    i64 b;

    if (a < 0)
    {
        b = (i64) (a - 0.6);
    }
    else
    {
        b = (i64) (a + 0.6);
    }

    return ((f64) b * pow(10, -precision));
}

/**
 * @brief 返回一个不小于整数x且为m的整数倍的最小值
 * @hideinitializer
 */
#ifndef GE_X_MULTI_M
#define GE_X_MULTI_M(x, m) (((x) + ((m) - 1)) / (m)) * (m)
#endif

/**
 * @brief 返回一个不大于整数x且为m的整数倍的最大值
 * @hideinitializer
 */
#ifndef LE_X_MULTI_M
#define LE_X_MULTI_M(x, m) ((x) / (m)) * (m)
#endif

/** @} Module_NumUtil */
/** @} Module_Base */

#endif // BASE_INC_NUM_UTIL_H_
