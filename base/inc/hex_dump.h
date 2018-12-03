/**
 * @file hex_dump.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_HEX_DUMP_H_
#define BASE_INC_HEX_DUMP_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_HexDump 打印二进制数据
 * @{
 */

#include <stddef.h>

/**
 * @brief 将一段二进制数据以字符串的形式打印出来
 * @param [out] buf 输出缓冲区
 * @param [in] buf_size 输出缓冲区的长度
 * @param [in] data 输入数据
 * @param [in] len 输入数据的长度
 * @return 返回输出数据的字节数，返回值<=0表示失败
 */
int HexDump(char* buf, size_t buf_size, const void* data, size_t len);

/** @} Module_HexDump */
/** @} Module_Base */

#endif // BASE_INC_HEX_DUMP_H_
