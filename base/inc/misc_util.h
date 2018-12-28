/**
 * @file misc_util.h
 * @brief 一些常用的宏和接口
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_MISC_UTIL_H_
#define BASE_INC_MISC_UTIL_H_

#include "data_types.h"

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_MiscUtil 杂项（宏、函数等）
 * @{
 */

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

/**
 * @brief 检查CPU的字节序是否为小端的。
 * @return 如果CPU的字节序为小端的则返回true，否则返回false。（小端Little Endian：低对低、高对高）
 * @note Linux系统中endian.h中有一个LITTLE_ENDIAN宏。
 */
#ifndef LITTLE_ENDIANED
#define LITTLE_ENDIANED\
    ({\
        union {\
            u16 d1; \
            u8 d2[2]; \
        } d; \
        const u16 high = UINT16_C(0x12); \
        const u16 low = UINT16_C(0x34); \
        d.d1 = ((high << 8) | low); \
        (d.d2[0] == UINT8_C(low)); \
    })
#endif // LITTLE_ENDIANED

/** @} Module_MiscUtil */
/** @} Module_Base */

#endif // BASE_INC_MISC_UTIL_H_
