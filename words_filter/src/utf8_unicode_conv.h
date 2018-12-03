/**
 * @file utf8_unicode_conv.h
 * @brief utf8编码与unicode编码之间的转换
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */
#ifndef WORDS_FILTER_SRC_UTF8_UNICODE_CONV_H_
#define WORDS_FILTER_SRC_UTF8_UNICODE_CONV_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_UTF8UnicodeConv UTF8UnicodeConv
 * @{
 */

#include "last_err_msg.h"

class UTF8UnicodeConv
{
public:
    UTF8UnicodeConv();
    ~UTF8UnicodeConv();

    /**
     * @param dst 目标缓冲区
     * @param dst_buf_size 目标缓冲区的字节数，要足够大，否则会报“errno: 7, err: Argument list too long”错误，dst的真实长度可以用wcslen等函数获取
     * @param src
     * @param src_bytes utf8编码的字符串长度，可以用strlen等函数获取
     */
    int UTF82Unicode(wchar_t* dst, size_t dst_buf_size, const char* src, size_t src_bytes);

    /**
     * @param dst dst 目标缓冲区
     * @param dst_buf_size 目标缓冲区的字节数，要足够大，否则会报“errno: 7, err: Argument list too long”错误，dst的真实长度可以用strlen等函数获取
     * @param src
     * @param src_bytes unicode编码的字符串长度，可以用wcslen等函数获取，记为src_len，则src_bytes=src_len*sizeof(wchar_t)
     */
    int Unicode2UTF8(char* dst, size_t dst_buf_size, const wchar_t* src, size_t src_bytes);

    const char* GetLastErrMsg() const
    {
        return last_err_msg_.What();
    }

private:
    LastErrMsg last_err_msg_;
};

/** @} Module_UTF8UnicodeConv */
/** @} Module_Base */

#endif // WORDS_FILTER_SRC_UTF8_UNICODE_CONV_H_
