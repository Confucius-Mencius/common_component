/**
 * @file words_filter_interface.h
 * @brief 脏字过滤引擎
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef WORDS_FILTER_INC_WORDS_FILTER_INTERFACE_H_
#define WORDS_FILTER_INC_WORDS_FILTER_INTERFACE_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_WordsFilter 脏字过滤
 * @{
 */

#include <stddef.h>
#include "module_interface.h"

class WordsFilterInterface : public ModuleInterface
{
public:
    /**
     * @brief 所支持的关键字的最大长度
     */
    static const int MAX_KEYWORDS_LEN = 32;

    /**
     * @brief unicode编码的关键字的最大长度
     * @attention 将从文件中加载的或动态添加的关键字转化成unicode编码后，如果超出这个长度，应该停止并报错
     */
    static const int MAX_UNICODE_KEYWORDS_LEN = 32;

    /**
     * @brief 输入字符串的最大长度
     */
    static const int MAX_INPUT_STR_LEN = 1024;

    /**
     * @brief 过滤时，将输入字符串转换成unicode编码后的最大长度
     */
    static const int MAX_UNICODE_INPUT_STR_LEN = MAX_INPUT_STR_LEN * 4;

public:
    virtual ~WordsFilterInterface()
    {
    }

    /**
     * @brief 加载关键字文件
     * @param file_path 关键字文件路径
     * @return 返回0表示成功，否则为错误码
     * @note utf8编码的keywords文件，第一行是忽略字符列表，都是单字符的；从第二行开始就是关键字，是单字符或者多字符的，每行一个关键字，只需配中文简体
     */
    virtual int LoadKeywordsFile(const char* file_path) = 0;

    /**
     * @brief 重新加载关键字文件
     * @param file_path 关键字文件路径
     * @return 返回0表示成功，否则为错误码
     */
    virtual int ReloadKeywordsFile(const char* file_path) = 0;

    /**
     * @brief 检查输入字符串中是否包含关键字
     * @param has_keywords 返回true表示包含关键字
     * @param src 输入字符串，必须是UTF8编码
     * @return 返回0表示成功，否则为错误码
     */
    virtual int Check(bool& has_keywords, const char* src) = 0;

    /**
     * @brief 对输入字符串进行关键字过滤，将关键字中的每个字符都替换成指定的符号
     * @param dst
     * @param dst_len
     * @param src
     * @param replace_symbol 替换符号
     * @return 返回0表示成功，否则为错误码
     */
    virtual int Filter(char** dst, size_t& dst_len, const char* src, char replace_symbol/* = '*'*/) = 0;

    /**
     * @brief 释放内存
     * @param dst
     */
    virtual void FilterRelease(char** dst) = 0;
};

/** @} Module_WordsFilter */
/** @} Module_Base */

#endif // WORDS_FILTER_INC_WORDS_FILTER_INTERFACE_H_
