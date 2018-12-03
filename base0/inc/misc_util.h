/**
 * @file misc_util.h
 * @brief 一些常用的宏和接口
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_MISC_UTIL_H_
#define BASE_INC_MISC_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_MiscUtil 杂项（宏、函数等）
 * @{
 */

#include <sys/types.h>

/**
 * @brief 声明私有的拷贝构造函数和赋值运算符，禁止类对象之间的相互拷贝、赋值。
 * @param [in] Type 一般为类名。
 * @hideinitializer
 */
#define DISALLOW_COPY_AND_ASSIGN(Type)\
private:\
    Type(const Type&);\
    Type& operator =(const Type&)

/**
 * @brief 声明私有的赋值运算符，禁止类对象之间的相互赋值。
 * @param [in] Type 一般为类名。
 * @hideinitializer
 */
#define DISALLOW_ASSIGN(Type)\
private:\
    Type& operator =(const Type&)

/**
 * @brief 声明保护型的构造函数，不允许实例化该类。
 * @param [in] Type 一般为类名。
 * @hideinitializer
 */
#define DISALLOW_INSTANCE(Type)\
protected:\
    Type()

/**
 * @brief Anti-warning macro...
 * @hideinitializer
 */
#define NOT_USED(x) ((void) x)

#if (defined(__linux__))
/*
 * from: /usr/src/linux-headers-3.2.0-23/include/linux/typecheck.h
 * Check at compile time that something is of a particular x.
 * Always evaluates to 1 so you may use it easily in comparisons.
 */
/**
 * @brief 变量类型检查。
 * @note 如果是两个不同类型的指针，则对它们做相等比较会报警或编译报错（-Wall时）。
 * @hideinitializer
 */
# define TYPE_CHECK(Type, x)\
({\
    Type dummy1;\
    typeof(x) dummy2;\
    (void) (&dummy1 == &dummy2);\
    1;\
})

/**
 * @brief TIME_AFTER宏来自内核，用于在溢出时比较自增的时间大小，不需要另外记录一个溢出标志，很巧妙。
 * @note time值一般用有符号数来表示，便于相减时不会溢出。这个宏不推荐使用，放在这里仅仅作为收藏。
 * @hideinitializer
 */
#define TIME_AFTER(time1, time2)\
    (TYPE_CHECK(unsigned long, time1)\
    && TYPE_CHECK(unsigned long, time2)\
    && ((long)(time2) - (long)(time1) < 0))

/**
 * @brief TIME_BEFORE
 * @hideinitializer
 */
#define TIME_BEFORE(time1, time2) TIME_AFTER(time2, time1)
#elif (defined(_WIN32) || defined(_WIN64))
#endif

/**
 * @brief 检查CPU的字节序是否为小端的。
 * @return 如果CPU的字节序为小端的则返回true，否则返回false。（小端Little Endian：低对低、高对高）
 * @note Linux系统中有个LITTLE_ENDIAN宏。
 */
bool IsLittleEndian();

/**
 * @brief 执行一个shell命令，将命令的输出写到一个字符串中。
 * @param [out] ret 命令的输出，如果不关心输出，传入NULL即可。
 * @param [in] cmd shell命令字符串，可以带各种选项和参数。
 * @return 返回true表示成功，否则表示失败。
 */
int ExecShellCmd(char* buf, int buf_size, const char* cmd);

/**
 * @brief 根据file_path和proj_id创建一个key值
 * @param file_path
 * @param proj_id
 * @return
 * @attention 要求file_path是绝对路径，不能是相对路径，否则结果是不确定的
 */
key_t FToKey(const char* file_path, char proj_id);

/** @} Module_MiscUtil */
/** @} Module_Base */

#endif // BASE_INC_MISC_UTIL_H_
