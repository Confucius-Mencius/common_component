/**
 * @file fast_crc32.h
 * @brief 计算crc32的接口
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_FAST_CRC32_H_
#define BASE_INC_FAST_CRC32_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_CRC32 CRC32
 * @{
 */

#include <stddef.h>

/**
 * @brief 快速计算crc32
 * @param [in] data
 * @param [in] length
 * @param [in] previous_crc32
 * @return crc32
 */
unsigned int FastCRC32(const void* data, size_t length, unsigned int previous_crc32 = 0);

/**
 * @brief 快速计算文件的crc32
 * @param [in] file_path
 * @return crc32
 */
unsigned int FastFileCRC32(const char* file_path);

/** @} Module_CRC32 */
/** @} Module_Base */

#endif // BASE_INC_FAST_CRC32_H_
