#include "words_filter.h"
#include <errno.h>
#include <fstream>

namespace words_filter
{
static const size_t LINE_LEN = 512;
static const size_t UNICODE_LINE_LEN = LINE_LEN * 4;

WordsFilter::WordsFilter() : last_err_msg_(), utf8_trad_simp_conv_(), utf8_unicode_conv_()
{
    winput_char_count_ = 0;
    no_ignore_char_count_ = 0;
}

WordsFilter::~WordsFilter()
{
}

const char* WordsFilter::GetVersion() const
{
    return NULL;
}

const char* WordsFilter::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

void WordsFilter::Release()
{
    delete this;
}

int WordsFilter::Initialize(const void* ctx)
{
    return 0;
}

void WordsFilter::Finalize()
{
    ignore_char_set_.clear();
    single_char_keywords_set_.clear();
    multi_char_keywords_set_.clear();
    multi_char_keywords_len_map_.clear();
    multi_char_keywords_last_char_set_.clear();
    keywords_char_pos_map_.clear();
    keywords_mark_list_.clear();
}

int WordsFilter::Activate()
{
    return 0;
}

void WordsFilter::Freeze()
{
}

int WordsFilter::LoadKeywordsFile(const char* file_path)
{
    if (NULL == file_path)
    {
        return -1;
    }

    std::ifstream ifs(file_path);
    if (!ifs.good())
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_,
                         "failed to open file " << file_path << ", errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (ifs.eof())
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "file " << file_path << " is empty");
        ifs.close();
        return -1;
    }

    // 第1行是忽略字符列表，都是单字符
    char line[LINE_LEN + 1] = "";
    ifs.getline(line, LINE_LEN);
    size_t line_len = (size_t) ifs.gcount();

    if (ProcessIgnoreLine(line, line_len) != 0)
    {
        ifs.close();
        return -1;
    }

    // 从第2行开始就是关键字了，每行一个
    while (!ifs.eof())
    {
        memset(line, 0, sizeof(line));
        ifs.getline(line, LINE_LEN);
        line_len = (size_t) ifs.gcount();

        if (line_len <= 0)
        {
            continue;
        }

        if (line_len > MAX_KEYWORDS_LEN)
        {
            SET_LAST_ERR_MSG(&last_err_msg_,
                             "keywords len: " << line_len << ", exceeds the limit: " << MAX_KEYWORDS_LEN);
            ifs.close();
            return -1;
        }

        if (ProcessKeywords(line, line_len) != 0)
        {
            ifs.close();
            return -1;
        }
    }

    ifs.close();
    return 0;
}

int WordsFilter::ReloadKeywordsFile(const char* file_path)
{
    if (NULL == file_path)
    {
        return -1;
    }

    ignore_char_set_.clear();
    single_char_keywords_set_.clear();
    multi_char_keywords_set_.clear();
    multi_char_keywords_len_map_.clear();
    multi_char_keywords_last_char_set_.clear();
    keywords_char_pos_map_.clear();

    return LoadKeywordsFile(file_path);
}

int WordsFilter::Check(bool& has_keywords, const char* src)
{
    if (NULL == src)
    {
        return -1;
    }

    std::string input_str(src);
    size_t src_len = input_str.length();

    if (src_len <= 0 || src_len > MAX_INPUT_STR_LEN)
    {
        SET_LAST_ERR_MSG(&last_err_msg_,
                         "src str: " << src << " , len: " << src_len << ", exceeds the limit: " << MAX_INPUT_STR_LEN);
        return -1;
    }

    char simp_input_str[MAX_INPUT_STR_LEN + 1] = "";
    size_t simp_input_str_len = MAX_INPUT_STR_LEN;

    if (utf8_trad_simp_conv_.Trad2Simp(simp_input_str, simp_input_str_len, src, src_len) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, utf8_trad_simp_conv_.GetLastErrMsg());
        return -1;
    }

    memset(winput_str_, 0, sizeof(winput_str_));
    size_t winput_str_buf_size = MAX_UNICODE_INPUT_STR_LEN;

    if (utf8_unicode_conv_.UTF82Unicode(winput_str_, winput_str_buf_size, simp_input_str, simp_input_str_len) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, utf8_unicode_conv_.GetLastErrMsg());
        return -1;
    }

    winput_char_count_ = wcsnlen(winput_str_, MAX_UNICODE_INPUT_STR_LEN);

    for (size_t i = 0; i < winput_char_count_; ++i)
    {
        winput_str_[i] = ToLower(winput_str_[i]);
    }

    no_ignore_char_count_ = 0;

    for (size_t i = 0; i < winput_char_count_; ++i)
    {
        // 去掉忽略字符
        if (ignore_char_set_.find(winput_str_[i]) != ignore_char_set_.end())
        {
            continue;
        }

        no_ignore_char_lower_zhs_input_str_[no_ignore_char_count_] = winput_str_[i];
        no_ignore_char_pos_[no_ignore_char_count_] = (int) i;
        ++no_ignore_char_count_;
    }

    has_keywords = Scan(no_ignore_char_lower_zhs_input_str_, no_ignore_char_count_);

    return 0;
}

int WordsFilter::Filter(char** dst, size_t& dst_len, const char* src, char replace_symbol)
{
    if (NULL == dst || NULL == src)
    {
        return -1;
    }

    bool has_keywords = false;

    if (Check(has_keywords, src) != 0)
    {
        return -1;
    }

    if (has_keywords)
    {
        char in[2] = "";
        in[0] = replace_symbol;
        size_t in_len = 1;

        wchar_t out[MAX_UNICODE_INPUT_STR_LEN + 1];
        memset(out, 0, sizeof(out));
        size_t out_buf_size = MAX_UNICODE_INPUT_STR_LEN;

        if (utf8_unicode_conv_.UTF82Unicode(out, out_buf_size, in, in_len) != 0)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, utf8_unicode_conv_.GetLastErrMsg());
            return -1;
        }

        size_t out_len = wcsnlen(out, MAX_UNICODE_INPUT_STR_LEN);
        if (out_len != 1)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "invalid replace symbol length: " << out_len);
            return -1;
        }

        for (KeywordsMarkList::const_iterator it = keywords_mark_list_.begin(); it != keywords_mark_list_.end(); ++it)
        {
            for (int i = it->begin; i < it->end; ++i)
            {
                winput_str_[no_ignore_char_pos_[i]] = out[0];
            }
        }
    }

    char* dst_buf = new char[MAX_INPUT_STR_LEN + 1];
    if (NULL == dst_buf)
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    memset(dst_buf, 0, MAX_INPUT_STR_LEN + 1);
    size_t dst_buf_size = MAX_INPUT_STR_LEN;

    if (utf8_unicode_conv_.Unicode2UTF8(dst_buf, dst_buf_size, winput_str_, winput_char_count_ * sizeof(wchar_t)) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, utf8_unicode_conv_.GetLastErrMsg());
        delete[] dst_buf;
        dst_buf = NULL;
        return -1;
    }

    *dst = dst_buf;
    dst_len = strnlen(dst_buf, MAX_INPUT_STR_LEN);

    return 0;
}

void WordsFilter::FilterRelease(char** dst)
{
    if (NULL == dst || NULL == *dst)
    {
        return;
    }

    delete[] (*dst);
}

int WordsFilter::ProcessIgnoreLine(const char* line, size_t line_len)
{
    char simp_line[LINE_LEN + 1] = "";
    size_t simp_line_len = LINE_LEN;

    if (utf8_trad_simp_conv_.Trad2Simp(simp_line, simp_line_len, line, line_len) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, utf8_trad_simp_conv_.GetLastErrMsg());
        return -1;
    }

    wchar_t wline[UNICODE_LINE_LEN + 1];
    memset(wline, 0, sizeof(wline));
    size_t wline_buf_size = UNICODE_LINE_LEN;

    if (utf8_unicode_conv_.UTF82Unicode(wline, wline_buf_size, simp_line, simp_line_len) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, utf8_unicode_conv_.GetLastErrMsg());
        return -1;
    }

    const size_t ignore_char_count = wcsnlen(wline, UNICODE_LINE_LEN);

    for (size_t i = 0; i < ignore_char_count; ++i)
    {
        if (AddIgnoreChar(ToLower(wline[i])) != 0)
        {
            return -1;
        }
    }

    return 0;
}

int WordsFilter::ProcessKeywords(const char* line, size_t line_len)
{
    char simp_line[LINE_LEN + 1] = "";
    size_t simp_line_len = LINE_LEN;

    if (utf8_trad_simp_conv_.Trad2Simp(simp_line, simp_line_len, line, line_len) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, utf8_trad_simp_conv_.GetLastErrMsg());
        return -1;
    }

    wchar_t wline[UNICODE_LINE_LEN + 1];
    memset(wline, 0, sizeof(wline));
    size_t wline_buf_size = UNICODE_LINE_LEN;

    if (utf8_unicode_conv_.UTF82Unicode(wline, wline_buf_size, simp_line, simp_line_len) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, utf8_unicode_conv_.GetLastErrMsg());
        return -1;
    }

    size_t keywords_len = wcsnlen(wline, UNICODE_LINE_LEN);
    if (keywords_len <= 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "empty unicode keywords: " << line);
        return -1;
    }

    if (keywords_len > MAX_UNICODE_KEYWORDS_LEN)
    {
        SET_LAST_ERR_MSG(&last_err_msg_,
                         "keywords len: " << keywords_len << ", exceeds the limit: " << MAX_UNICODE_KEYWORDS_LEN);
        return -1;
    }

    for (size_t i = 0; i < keywords_len; ++i)
    {
        wline[i] = ToLower(wline[i]);
    }

    if (AddKeywords(wline, keywords_len, line) != 0)
    {
        return -1;
    }

    return 0;
}

int WordsFilter::AddIgnoreChar(wchar_t ignore_char)
{
    IgnoreCharSet::const_iterator it = ignore_char_set_.find(ignore_char);
    if (it != ignore_char_set_.end())
    {
        char dst[8] = "";
        size_t dst_buf_size = sizeof(dst);

        if (WcharToUtf8(dst, dst_buf_size, ignore_char) != 0)
        {
            return -1;
        }

        SET_LAST_ERR_MSG(&last_err_msg_, "ignore char " << dst << " already exist");
        return -1;
    }

    if (!ignore_char_set_.insert(ignore_char).second)
    {
        char dst[8] = "";
        size_t dst_buf_size = sizeof(dst);

        if (WcharToUtf8(dst, dst_buf_size, ignore_char) != 0)
        {
            return -1;
        }

        SET_LAST_ERR_MSG(&last_err_msg_, "failed to insert to set, ignore char: " << dst);
        return -1;
    }

    return 0;
}

int WordsFilter::AddKeywords(const wchar_t* keywords, size_t keywords_len, const char* utf8_keywords)
{
    if (keywords_len > 1)
    {
        MultiCharKeywordsSet::const_iterator it = multi_char_keywords_set_.find(keywords);
        if (it != multi_char_keywords_set_.end())
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "unicode keywords " << utf8_keywords << " already exist");
            return -1;
        }

        if (!multi_char_keywords_set_.insert(keywords).second)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "failed to insert to set, unicode keywords: " << utf8_keywords);
            return -1;
        }

        multi_char_keywords_len_map_[keywords[0]] |= (1 << (keywords_len - 2));
        multi_char_keywords_last_char_set_.insert(keywords[keywords_len - 1]);

        for (size_t i = 0; i < keywords_len; ++i)
        {
            keywords_char_pos_map_[keywords[i]] |= (1 << i);
        }
    }
    else // 1 == keywords_len
    {
        SingleCharKeywordsSet::const_iterator it = single_char_keywords_set_.find(keywords[0]);
        if (it != single_char_keywords_set_.end())
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "unicode keywords " << utf8_keywords << " already exist");
            return -1;
        }

        if (!single_char_keywords_set_.insert(keywords[0]).second)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "failed to insert to set, unicode keywords: " << keywords);
            return -1;
        }

        keywords_char_pos_map_[keywords[0]] |= 1;
    }

    return 0;
}

bool WordsFilter::Scan(const wchar_t* no_ignore_char_input_str, size_t no_ignore_char_input_str_len)
{
    keywords_mark_list_.clear();

    bool has_keywords = false;
    size_t n = 0;
    KeywordsMark keywords_mark;

    while (n < no_ignore_char_input_str_len)
    {
        size_t step = 1;

        if (!IsKeywordsCharAtPos(no_ignore_char_input_str[n], 0))
        {
            n += step;
            continue;
        }

        wchar_t begin = no_ignore_char_input_str[n];

        if (single_char_keywords_set_.find(begin) != single_char_keywords_set_.end())
        {
            keywords_mark.begin = n;
            keywords_mark.end = n + 1;
            keywords_mark_list_.push_back(keywords_mark);

            has_keywords = true;
            n += step;

            continue;
        }

        size_t left_char_count = no_ignore_char_input_str_len - 1 - n;
        for (size_t i = 1; i <= left_char_count; ++i)
        {
            wchar_t wchar = no_ignore_char_input_str[n + i];

            if ((!IsKeywordsCharAtPos(wchar, 0)) && (step == i))
            {
                ++step;
            }

            if (!IsKeywordsCharAtPos(wchar, i))
            {
                break;
            }

            if (IsSuspectedMultiCharKeywords(begin, wchar, i + 1))
            {
                wchar_t words[MAX_UNICODE_KEYWORDS_LEN + 1];
                wcsncpy(words, no_ignore_char_input_str + n, i + 1);
                words[i + 1] = L'\0';

                if (multi_char_keywords_set_.find(words) != multi_char_keywords_set_.end())
                {
                    keywords_mark.begin = n;
                    keywords_mark.end = n + i + 1;
                    keywords_mark_list_.push_back(keywords_mark);

                    has_keywords = true;
                    break;
                }
            }
        }

        n += step;
    }

    return has_keywords;
}

bool WordsFilter::IsKeywordsCharAtPos(wchar_t wchar, int pos) const
{
    KeywordsCharPosMap::const_iterator it = keywords_char_pos_map_.find(wchar);
    if (it == keywords_char_pos_map_.end())
    {
        return false;
    }

    return ((it->second & (1 << pos)) != 0);
}

bool WordsFilter::IsSuspectedMultiCharKeywords(wchar_t first, wchar_t last, int len) const
{
    MultiCharKeywordsLenMap::const_iterator it = multi_char_keywords_len_map_.find(first);
    if (it == multi_char_keywords_len_map_.end())
    {
        return false;
    }

    if ((it->second & (1 << (len - 2))) != 0)
    {
        return multi_char_keywords_last_char_set_.find(last) != multi_char_keywords_last_char_set_.end();
    }

    return false;
}
}
