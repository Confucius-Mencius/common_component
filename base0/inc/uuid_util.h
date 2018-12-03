/**
 * @file uuid_util.h
 * @brief UUID/GUID
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_UUID_UTIL_H_
#define BASE_INC_UUID_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_UUID 生成UUID的方法
 * @{
 */

#include <string>

#if (defined(__linux__))

#include "uuid/uuid.h"

#define UUID_LEN_38 38
#define UUID_LEN_32 32

typedef struct
{
    unsigned int Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} Uuid;
#elif (defined(_WIN32) || defined(_WIN64))
#include <objbase.h>
typedef GUID Uuid;
#endif

/**
 * @brief 生成UUID
 * @return
 */
Uuid MakeUuid();

/**
 * @brief 获取UUID的字符串格式，长度为38个字符（短线间隔）
 * @param buf char buf[UUID_LEN_38 + 1]
 * @param buf_size
 * @param uuid
 * @return
 */
char* Uuid38(char* buf, size_t buf_size, const Uuid& uuid);

/**
 * @brief 获取UUID的字符串格式，长度为32个字符
 * @param buf char buf[UUID_LEN_32 + 1]
 * @param buf_size
 * @param uuid
 * @return
 */
char* Uuid32(char* buf, size_t buf_size, const Uuid& uuid);

/** @} Module_UUID */
/** @} Module_Base */

#endif // BASE_INC_UUID_UTIL_H_
