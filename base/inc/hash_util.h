/**
 * @file hash_util.h
 * @brief hash util
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_HASH_UTIL_H_
#define BASE_INC_HASH_UTIL_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_HashUtil 一些hash算法
 * @{
 */

#include <stddef.h>

/**
 * @brief
 * @param [in] data
 * @param [in] len
 * @return
 */
unsigned long PJWHash(const char* data, size_t len);

/** @} Module_HashUtil */
/** @} Module_Base */

#endif // BASE_INC_HASH_UTIL_H_
