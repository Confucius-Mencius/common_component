/**
 * @file data_type.h
 * @brief 定义常用数据类型
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_DATA_TYPE_H_
#define BASE_INC_DATA_TYPE_H_

#include <float.h>
#include <limits.h>
#include <stddef.h>
#include "std_int.h"

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_DataType 跨平台数据类型
 * @{
 */

/**
 * @brief 字节类型。
 * @attention 作为函数参数时，缓冲区指针一般用void*，不用byte*
 */
typedef unsigned char byte;

#if (defined(__linux__))
/**
 * @brief 8位有符号整数类型
 */
typedef int8_t i8;

/**
 * @brief 8位无符号整数类型
 */
typedef uint8_t u8;

/**
 * @brief 16位有符号整数类型
 */
typedef int16_t i16;

/**
 * @brief 16位无符号整数类型
 */
typedef uint16_t u16;

/**
 * @brief 32位有符号整数类型
 */
typedef int32_t i32;

/**
 * @brief 32位无符号整数类型
 */
typedef uint32_t u32;

/**
 * @brief 64位有符号整数类型
 */
typedef int64_t i64;

/**
 * @brief 64位无符号整数类型
 */
typedef uint64_t u64;
#elif (defined(_WIN32) || defined(_WIN64))
#include <Windows.h>
typedef char i8;
typedef unsigned char u8;
typedef short i16;
typedef unsigned short u16;
typedef int i32;
typedef unsigned int u32;
typedef __int64 i64;
typedef unsigned __int64 u64;
#endif /* */

/**
 * @brief 无符号长整数类型
 */
typedef unsigned long ulong;

/**
 * @brief 32位的单精度浮点数类型
 */
typedef float f32;

/**
 * @brief 64位的双精度浮点数类型
 */
typedef double f64;

/**
 * @brief 96位的双精度浮点数类型
 */
typedef long double f96;

/**
 * @brief i8类型的最大值
 * @hideinitializer
 */
#define I8_MAX SCHAR_MAX

/**
 * @brief i8类型的最小值
 * @hideinitializer
 */
#define I8_MIN SCHAR_MIN

/**
 * @brief u8类型的最大值
 * @hideinitializer
 */
#define U8_MAX UCHAR_MAX

/**
 * @brief u8类型的最小值
 * @hideinitializer
 */
#define U8_MIN UINT8_C(0)

/**
 * @brief i16类型的最大值
 * @hideinitializer
 */
#define I16_MAX SHRT_MAX

/**
 * @brief i16类型的最小值
 * @hideinitializer
 */
#define I16_MIN SHRT_MIN

/**
 * @brief u16类型的最大值
 * @hideinitializer
 */
#define U16_MAX USHRT_MAX

/**
 * @brief u16类型的最小值
 * @hideinitializer
 */
#define U16_MIN UINT16_C(0)

/**
 * @brief i32类型的最大值
 * @hideinitializer
 */
#define I32_MAX INT_MAX

/**
 * @brief i32类型的最小值
 * @hideinitializer
 */
#define I32_MIN INT_MIN

/**
 * @brief u32类型的最大值
 * @hideinitializer
 */
#define U32_MAX UINT_MAX

/**
 * @brief u32类型的最小值
 * @hideinitializer
 */
#define U32_MIN UINT32_C(0)

/**
 * @brief i64类型的最大值
 * @hideinitializer
 */
#define I64_MAX INT64_C(0x7fffffffffffffff)

/**
 * @brief i64类型的最小值
 * @hideinitializer
 */
#define I64_MIN (-INT64_C(0x7fffffffffffffff) - 1)

/**
 * @brief u64类型的最大值
 * @hideinitializer
 */
#define U64_MAX UINT64_C(0xffffffffffffffff)

/**
 * @brief u64类型的最小值
 * @hideinitializer
 */
#define U64_MIN UINT64_C(0)

//#define LONG_MAX LONG_MAX // LONG_MAX已经定义
//#define LONG_MIN LONG_MIN // LONG_MIN已经定义
//#define ULONG_MAX ULONG_MAX // ULONG_MAX已经定义
#define ULONG_MIN 0UL

/**
 * @brief float32类型的最大值
 * @hideinitializer
 */
#define F32_MAX FLT_MAX

/**
 * @brief float32类型的最小值
 * @hideinitializer
 */
#define F32_MIN -(F32_MAX) // FLT_MIN是正的最小值

/**
 * @brief float64类型的最大值
 * @hideinitializer
 */
#define F64_MAX DBL_MAX

/**
 * @brief float64类型的最小值
 * @hideinitializer
 */
#define F64_MIN -(F64_MAX) // DBL_MIN是正的最小值

/**
 * @brief float96类型的最大值
 * @hideinitializer
 */
#define F96_MAX LDBL_MAX

/**
 * @brief float96类型的最小值
 * @hideinitializer
 */
#define F96_MIN -(F96_MAX) // LDBL_MIN是正的最小值

/**
 * @brief 输入参数修饰符
 * @hideinitializer
 */
#ifndef IN
#define IN
#endif

/**
 * @brief 输出参数修饰符
 * @hideinitializer
 */
#ifndef OUT
#define OUT
#endif

/**
 * @brief 输入+输出参数修饰符
 * @hideinitializer
 */
#ifndef INOUT
#define INOUT
#endif

/** @} Module_DataType */
/** @} Module_Base */

#endif // BASE_INC_DATA_TYPE_H_
