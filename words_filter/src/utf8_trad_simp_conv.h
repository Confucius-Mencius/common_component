/**
 * @file utf8_trad_simp_conv.h
 * @brief utf8编码的中文简繁体转换
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef WORDS_FILTER_SRC_UTF8_TRAD_SIMP_CONV_H_
#define WORDS_FILTER_SRC_UTF8_TRAD_SIMP_CONV_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_UTF8TradSimpConv UTF8TradSimpConv
 * @{
 */

#include "last_err_msg.h"

// utf8中文字符（简繁体）所占的字节数：
// 占2个字节的：〇
// 占3个字节的：基本等同于GBK，含21000多个汉字
// 占4个字节的：中日韩超大字符集里面的汉字，有5万多个

class UTF8TradSimpConv
{
public:
    UTF8TradSimpConv();
    ~UTF8TradSimpConv();

    /**
     * @param dst dst[dst_buf_size]
     * @param dst_len 作为输入参数时要足够大，会输出dst的真实长度
     * @param src
     * @param src_len utf8编码的字符串长度，可以用strlen等函数获取
     */
    int Trad2Simp(char* dst, size_t& dst_len, const char* src, size_t src_len);

    /**
     * @param dst dst[dst_buf_size]
     * @param dst_len 作为输入参数时要足够大，会输出dst的真实长度
     * @param src
     * @param src_len utf8编码的字符串长度，可以用strlen等函数获取
     */
    int Simp2Trad(char* dst, size_t& dst_len, const char* src, size_t src_len);

    const char* GetLastErrMsg() const
    {
        return last_err_msg_.What();
    }

private:
    LastErrMsg last_err_msg_;
};

/** @} Module_UTF8TradSimpConv */
/** @} Module_Base */

#endif // WORDS_FILTER_SRC_UTF8_TRAD_SIMP_CONV_H_
