/**
 * @file base64_util.h
 * @brief base64编解码接口
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_BASE64_UTIL_H_
#define BASE_INC_BASE64_UTIL_H_

// base64中的ascii码：A-Z a-z 0-9 + / =，共65个

/** 
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_Base64Util Base64Util
 * @{
 */

#include <stddef.h>

/**
 * @brief Base64编码
 * @param [out] out_data 加密后的数据
 * @param [out] out_data_len 加密后的数据字节数
 * @param [in] in_data 要加密的数据
 * @param [in] in_data_len 要加密的数据字节数
 * @param [in] with_newline 是否换行。编码后的格式控制，每64个字符增加一个换行
 * @return =0表示成功，否则失败
 * @attention out_data使用完毕需要调用Base64Release释放内存
 */
int Base64Encode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                 int with_newline);

/**
 * @brief Base64解码
 * @param [out] out_data 解密后的数据
 * @param [out] out_data_len 解密后的数据字节数
 * @param [in] in_data 要解密的数据
 * @param [in] in_data_len 要解密的数据字节数
 * @param [in] with_newline @see Encode
 * @return =0表示成功，否则失败
 * @attention out_data使用完毕需要调用Base64Release释放内存
 */
int Base64Decode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                 int with_newline);

/**
 * @brief 清理Encode/Decode的内存
 * @details Encode/Decode获得的out_data在使用完毕后需要调用该接口清理内存
 * @param [in,out] out_data
 */
void Base64Release(unsigned char** out_data);

/** @} Module_Base64Util */
/** @} Module_Base 基础库 */

#endif // BASE_INC_BASE64_UTIL_H_
