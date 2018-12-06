/**
 * @file num_str_util.h
 * @brief 数字和字符串之间的转换
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */
#ifndef BASE_INC_NUM_STR_UTIL_H_
#define BASE_INC_NUM_STR_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_NumStrUtil NumStrUtil
 * @{
 */

#include <stdlib.h>
#include "data_type.h"
#include "misc_util.h"
#include "str_util.h"
#include "type2type.h"

/**
 *
 * @param buf
 * @param buf_size
 * @param num
 * @return 数字字符串的长度
 */
int Num2Str(char* buf, int buf_size, Type2Type<i32>, i32 num)
{
    return StrPrintf(buf, buf_size, "%d", num);
}

/**
 *
 * @param buf
 * @param buf_size
 * @param num
 * @return 数字字符串的长度
 */
int Num2Str(char* buf, int buf_size, Type2Type<u32>, u32 num)
{
    return StrPrintf(buf, buf_size, "%u", num);
}

/**
 *
 * @param buf
 * @param buf_size
 * @param num
 * @return 数字字符串的长度
 */
int Num2Str(char* buf, int buf_size, Type2Type<i64>, i64 num)
{
    return StrPrintf(buf, buf_size, "%ld", num);
}

/**
 *
 * @param buf
 * @param buf_size
 * @param num
 * @return 数字字符串的长度
 */
int Num2Str(char* buf, int buf_size, Type2Type<u64>, u64 num)
{
    return StrPrintf(buf, buf_size, "%lu", num);
}

/**
 *
 * @param buf
 * @param buf_size
 * @param num
 * @return 数字字符串的长度
 * @attention 最多保留6位小数
 */
int Num2Str(char* buf, int buf_size, Type2Type<f32>, f32 num)
{
    return StrPrintf(buf, buf_size, "%f", num);
}

/**
 *
 * @param buf
 * @param buf_size
 * @param num
 * @return 数字字符串的长度
 * @attention 最多保留6位小数
 */
int Num2Str(char* buf, int buf_size, Type2Type<f64>, f64 num)
{
    return StrPrintf(buf, buf_size, "%f", num);
}

/**
 *
 * @param str 不能为空指针
 * @return
 * @attention 只支持十进制
 */
i32 Str2Num(Type2Type<i32>, const char* str)
{
    return atoi(str);
}

/**
 *
 * @param str 不能为空指针
 * @return
 * @attention 只支持十进制
 */
u32 Str2Num(Type2Type<u32>, const char* str)
{
    return atoi(str);
}

/**
 *
 * @param str 不能为空指针
 * @return
 * @attention 只支持十进制
 */
i64 Str2Num(Type2Type<i64>, const char* str)
{
    return atoll(str);
}

/**
 *
 * @param str 不能为空指针
 * @return
 * @attention 只支持十进制
 */
u64 Str2Num(Type2Type<u64>, const char* str)
{
    return atoll(str);
}

/**
 *
 * @param str 不能为空指针
 * @return
 * @attention 只支持十进制
 */
f32 Str2Num(Type2Type<f32>, const char* str)
{
    return (f32) atof(str);
}

/**
 *
 * @param str 不能为空指针
 * @return
 * @attention 只支持十进制
 */
f64 Str2Num(Type2Type<f64>, const char* str)
{
    return atof(str);
}

/** @} Module_NumStrUtil */
/** @} Module_Base */

#endif // BASE_INC_NUM_STR_UTIL_H_
