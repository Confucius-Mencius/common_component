/**
 * @file md5_util.h
 * @brief 计算MD5的接口
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_MD5_UTIL_H_
#define BASE_INC_MD5_UTIL_H_

#include "openssl/md5.h"

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_MD5Util MD5Util
 * @{
 */

#include <string>

/**
 * @breif MD5字符串的长度
 * @hideinitializer
 */
#define MD5_STR_LEN 32

/**
 * @brief 计算一个字符串或一段二进制数据的md5值，返回可打印的md5字符串
 * @param [out] buf 存放md5的buf
 * @param [in] buf_size
 * @param [in] data 输入数据
 * @param [in] len
 * @return 返回md5字符串的长度，返回值<=0表示失败
 */
int CalcMD5(char buf[MD5_STR_LEN + 1], size_t buf_size, const unsigned char* data, size_t len);

/**
 * @brief 计算文件的md5，返回可打印的md5字符串
 * @param [out] buf 存放md5的buf
 * @param [in] buf_size
 * @param [in] file_path 文件路径
 * @return 返回md5字符串的长度，返回值<=0表示失败
 */
int CalcFileMD5(char buf[MD5_STR_LEN + 1], size_t buf_size, const char* file_path);

/**
 * @brief 计算一个字符串或一段二进制数据的md5摘要，返回一段二进制数据
 * @param md
 * @param buf_size
 * @param data
 * @param len
 * @return =0表示成功，否则失败
 */
int CalcMD5Digest(unsigned char md[MD5_DIGEST_LENGTH], size_t buf_size, const unsigned char* data, size_t len);

/**
 * @brief 计算文件的md5摘要，返回一段二进制数据
 * @param md
 * @param buf_size
 * @param file_path
 * @return =0表示成功，否则失败
 */
int CalcFileMD5Digest(unsigned char md[MD5_DIGEST_LENGTH], size_t buf_size, const char* file_path);

/** @} Module_MD5Util */
/** @} Module_Base */

#endif // BASE_INC_MD5_UTIL_H_
