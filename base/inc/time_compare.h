#ifndef BASE_INC_TIME_COMPARE_H_
#define BASE_INC_TIME_COMPARE_H_

#if (defined(__linux__))
/*
 * from: /usr/src/linux-headers-3.2.0-23/include/linux/TYPE_CHECK.h
 * Check at compile time that something is of a particular x.
 * Always evaluates to 1 so you may use it easily in comparisons.
 */
/**
 * @brief 变量类型检查。
 * @note 如果是两个不同类型的指针，则对它们做相等比较会报警或编译报错（-Wall时）。\n
 *       编译选项：-std=gnu++11
 * @hideinitializer
 */
#if __cplusplus >= 201103L
#define TYPE_CHECK(Type, x)\
    ({\
        Type dummy1;\
        decltype(x) dummy2;\
        (void) (&dummy1 == &dummy2);\
        1;\
    })
#else
#define TYPE_CHECK(Type, x)\
    ({\
        Type dummy1;\
        typeof(x) dummy2;\
        (void) (&dummy1 == &dummy2);\
        1;\
    })
#endif // __cplusplus >= 201103L

/**
 * @brief TIME_AFTER 时间上 a > b
 * @details TIME_AFTER宏来自内核，用于在溢出时比较自增的时间大小，不需要另外记录一个溢出标志，很巧妙。\n
 *          首先确保两个输入参数a和b的数据类型为unsigned long，且. 两个值之间相差从逻辑值来讲应<=long整型的最大值（两个值之间的空位应<=LONG_MAX)。
 * @hideinitializer
 */
#define TIME_AFTER(a, b)\
    (TYPE_CHECK(unsigned long, a)\
     && TYPE_CHECK(unsigned long, b)\
     && ((long)(b) - (long)(a) < 0))

/**
 * @brief TIME_BEFORE 时间上 a < b
 * @hideinitializer
 */
#define TIME_BEFORE(a, b) TIME_AFTER(b, a)

/**
 * @brief TIME_AFTER_EQ 时间上 a >= b
 * @hideinitializer
 */
#define TIME_AFTER_EQ(a, b)\
    (TYPE_CHECK(unsigned long, a)\
     && TYPE_CHECK(unsigned long, b)\
     &&((long)(a) - (long)(b) >= 0))

/**
 * @brief TIME_BEFORE_EQ 时间上 a <= b
 * @hideinitializer
 */
#define TIME_BEFORE_EQ(a, b) TIME_AFTER_EQ(b, a)

/**
 * @brief TIME_IN_RANGE 时间上 b <= a <= c
 * @hideinitializer
 */
#define TIME_IN_RANGE(a, b, c)\
    (TIME_AFTER_EQ(a, b) && TIME_BEFORE_EQ(a, c))

#elif (defined(_WIN32) || defined(_WIN64))
// windows系统上暂无实现
#endif

#endif // BASE_INC_TIME_COMPARE_H_
