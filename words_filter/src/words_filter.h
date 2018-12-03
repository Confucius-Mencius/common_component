#ifndef WORDS_FILTER_SRC_WORDS_FILTER_H_
#define WORDS_FILTER_SRC_WORDS_FILTER_H_

#include <list>
#include <map>
#include <set>
#include <string.h>
#include "last_err_msg.h"
#include "utf8_trad_simp_conv.h"
#include "utf8_unicode_conv.h"
#include "words_filter_interface.h"

// 转换步骤：utf8繁体->utf8简体->unicode->小写

namespace words_filter
{
class WordsFilter : public WordsFilterInterface
{
public:
    WordsFilter();
    virtual ~WordsFilter();

    ///////////////////////// ModuleInterface /////////////////////////
    virtual const char* GetVersion() const;
    const char* GetLastErrMsg() const;
    virtual void Release();
    virtual int Initialize(const void* ctx);
    virtual void Finalize();
    virtual int Activate();
    virtual void Freeze();

    ///////////////////////// WordsFilterInterface /////////////////////////
    virtual int LoadKeywordsFile(const char* file_path);
    virtual int ReloadKeywordsFile(const char* file_path);
    virtual int Check(bool& has_keywords, const char* src);
    virtual int Filter(char** dst, size_t& dst_len, const char* src, char replace_symbol);
    virtual void FilterRelease(char** dst);

private:
    int ProcessIgnoreLine(const char* line, size_t line_len);
    int ProcessKeywords(const char* line, size_t line_len);

    wchar_t ToLower(wchar_t wchar)
    {
        return (wchar_t) towlower(wchar);
    }

    int WcharToUtf8(char* dst, size_t dst_buf_size, wchar_t wchar)
    {
        wchar_t src[2];

        memset(src, 0, sizeof(src));
        src[0] = wchar;
        size_t src_len = wcslen(src);

        if (utf8_unicode_conv_.Unicode2UTF8(dst, dst_buf_size, src, src_len) != 0)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, utf8_unicode_conv_.GetLastErrMsg());
            return -1;
        }

        return 0;
    }

    int AddIgnoreChar(wchar_t ignore_char);
    int AddKeywords(const wchar_t* keywords, size_t keywords_len, const char* utf8_keywords);

    bool Scan(const wchar_t* no_ignore_char_input_str,
              size_t no_ignore_char_input_str_len); // 如果输入串中有至少一个关键字则返回true，否则返回false
    bool IsKeywordsCharAtPos(wchar_t wchar, int pos) const; // 判断字符是否为某个关键字中指定pos上的字符
    bool IsSuspectedMultiCharKeywords(wchar_t first, wchar_t last, int len) const; // 判断参数确定的字符串是否可能是一个关键字

private:
    typedef std::set<wchar_t> IgnoreCharSet;
    IgnoreCharSet ignore_char_set_; // 忽略字符集合

    typedef std::set<wchar_t> SingleCharKeywordsSet;
    SingleCharKeywordsSet single_char_keywords_set_; // 单字符关键字集合

    typedef std::set<std::wstring> MultiCharKeywordsSet;
    MultiCharKeywordsSet multi_char_keywords_set_; // 多字符关键字集合

    // 记录多字符关键字的长度信息，key为多字符关键字中第一个字符
    // 例如以“江”字打头的关键字有“江青”和“江泽民”，则multi_char_keywords_len_map_["江"]的值为：11
    // 11（第0位为1，第1位为1）表示在读到“江”字的时候，如果再读1个字符就可以得到一个关键字；如果再读两个字符，也可以得到一个关键字
    typedef std::map<wchar_t, int> MultiCharKeywordsLenMap;
    MultiCharKeywordsLenMap multi_char_keywords_len_map_;

    typedef std::set<wchar_t> MultiCharKeywordsLastCharSet;
    MultiCharKeywordsLastCharSet multi_char_keywords_last_char_set_; // 多字符关键字中的最后一个字符

    // 记录关键字的每个字符在其所属关键字中的pos，key为关键字中的字符
    // 例如关键字“李鹏”的长度为2，pos为0的字符是‘李’，pos为1的字符是‘鹏’，
    // 关键字“李小鹏”的长度为3，pos为0的字符是‘李’，pos为1的字符是‘小’，pos为2的字符是‘鹏’，
    // 则keywords_char_pos_map_["鹏"]的值为：110
    // 110（第0位为0，第1位为1，第2位为1）表示‘鹏’字在有些关键字中的pos为1，在有些关键字中的pos为2
    typedef std::map<wchar_t, int> KeywordsCharPosMap;
    KeywordsCharPosMap keywords_char_pos_map_;

    // 将输入字符串转换成unicode编码后的字符串（未转换成小写简体中文）
    wchar_t winput_str_[MAX_UNICODE_INPUT_STR_LEN + 1];
    size_t winput_char_count_;

    // 去掉输入字符串中的忽略字符后的字符串及各个字符在原始字符串中的pos
    wchar_t no_ignore_char_lower_zhs_input_str_[MAX_UNICODE_INPUT_STR_LEN + 1];
    int no_ignore_char_count_;
    int no_ignore_char_pos_[MAX_UNICODE_INPUT_STR_LEN + 1];

    struct KeywordsMark
    {
        int begin; // 关键字在原始字符串中的起始pos
        int end; // 关键字在原始字符串中的截止pos（关键字最后一个字符的下一个位置）
    };

    // 输入字符串中的关键字标记列表
    typedef std::list<KeywordsMark> KeywordsMarkList;
    KeywordsMarkList keywords_mark_list_;

    LastErrMsg last_err_msg_;
    UTF8TradSimpConv utf8_trad_simp_conv_;
    UTF8UnicodeConv utf8_unicode_conv_;
};
}

#endif // WORDS_FILTER_SRC_WORDS_FILTER_H_
