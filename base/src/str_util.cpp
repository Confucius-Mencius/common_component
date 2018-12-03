#include "str_util.h"
#include <string.h>
#include <stdarg.h>

int StrLen(const char* str, size_t buf_size)
{
    if (NULL == str || buf_size <= 1)
    {
        return 0;
    }

    return (int) strnlen(str, buf_size - 1);
}

char* StrCpy(char* buf, size_t buf_size, const char* str)
{
    if (NULL == buf || buf_size <= 1 || NULL == str)
    {
        return NULL;
    }

    const size_t n = buf_size - 1;
    strncpy(buf, str, n);
    buf[n] = '\0';

    return buf;
}

char* StrCat(char* buf, size_t buf_size_left, const char* str)
{
    if (NULL == buf || buf_size_left <= 1 || NULL == str)
    {
        return NULL;
    }

    const size_t n = buf_size_left - 1;
    strncat(buf, str, n);

    return buf;
}

bool StrCaseEQ(const char* str1, const char* str2, size_t min_buf_size)
{
    if (NULL == str1 || NULL == str2 || min_buf_size <= 1)
    {
        return false;
    }

    return (0 == strncmp(str1, str2, min_buf_size));
}

bool StrNoCaseEQ(const char* str1, const char* str2, size_t min_buf_size)
{
    if (NULL == str1 || NULL == str2 || min_buf_size <= 1)
    {
        return false;
    }

    return (0 == strncasecmp(str1, str2, min_buf_size));
}

bool StrCaseBeginWith(const char* str, const char* needle)
{
    if (NULL == str)
    {
        return false;
    }

    const char* p = strstr(str, needle);
    if (p != str)
    {
        return false;
    }

    return true;
}

bool StrNoCaseBeginWith(const char* str, const char* needle)
{
    if (NULL == str)
    {
        return false;
    }

    const char* p = strcasestr(str, needle);
    if (p != str)
    {
        return false;
    }

    return true;
}

bool StrCaseEndWith(const char* str, const char* needle)
{
    if (NULL == str)
    {
        return false;
    }

    const char* p = strstr(str, needle);
    if (NULL == p)
    {
        return false;
    }

    const char* end = p + strlen(p);
    if (end[0] != '\0')
    {
        return false;
    }

    return true;
}

bool StrNoCaseEndWith(const char* str, const char* needle)
{
    if (NULL == str)
    {
        return false;
    }

    const char* p = strcasestr(str, needle);
    if (NULL == p)
    {
        return false;
    }

    const char* end = p + strlen(p);
    if (end[0] != '\0')
    {
        return false;
    }

    return true;
}

int StrPrintf(char* buf, size_t buf_size, const char* fmt, ...)
{
    if (NULL == buf || buf_size <= 1 || NULL == fmt)
    {
        return 0;
    }

    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(buf, buf_size, fmt, ap);
    va_end(ap);

    if (n < 0)
    {
        return -1;
    }

    // (n >= buf_size)时意味着空间不够，结果被截断
    if (n >= (int) buf_size)
    {
        n = (int) (buf_size - 1);
    }

    buf[n] = '\0';
    return n;
}

int StrTrim(char* buf, int buf_size, const char* str, int len, const char* delims)
{
    std::string result;
    result.assign(str, len);

    std::string::size_type pos = result.find_last_not_of(delims);
    if (pos != std::string::npos)
    {
        result.erase(++pos);
    }

    pos = result.find_first_not_of(delims);
    if (pos != std::string::npos)
    {
        result.erase(0, pos);
    }
    else
    {
        result.erase();
    }

    return StrPrintf(buf, buf_size, "%s", result.c_str());
}

int StrReplace(char* buf, int buf_size, const char* str, const char* search, const char* replace)
{
    const std::string search_s(search);
    const std::string replace_s(replace);

    std::string result(str);

    std::string::size_type pos = result.find(search_s);

    while (pos != std::string::npos)
    {
        result = result.replace(pos, search_s.size(), replace_s);
        pos = pos - search_s.size() + replace_s.size() + 1;
        pos = result.find(search_s, pos);
    }

    return StrPrintf(buf, buf_size, "%s", result.c_str());
}

void StrReverse(char str[], int len)
{
    int i, j;
    char c;

    for (i = 0, j = len - 1; i < j; ++i, --j)
    {
        c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
}

int MatchWithAsteriskW(const char* str, int len1, const char* pattern, int len2)
{
    if (NULL == str || NULL == pattern)
    {
        return -1;
    }

    int mark = 0; // 用于分段标记，'*'分隔的字符串
    int p1 = 0, p2 = 0;

    while (p1 < len1 && p2 < len2)
    {
        if (pattern[p2] == '?')
        {
            p1++;
            p2++;
            continue;
        }

        if (pattern[p2] == '*')
        {
            // 如果当前是*号，则mark前面一部分已经获得匹配，从当前点开始继续下一个块的匹配
            p2++;
            mark = p2;
            continue;
        }

        if (str[p1] != pattern[p2])
        {
            if (p1 == 0 && p2 == 0)
            {
                // 如果是首字符，特殊处理，不相同即匹配失败
                return -1;
            }

            /*
            * pattern: ...*bdef*...
            *              ^
            *             mark
            *                ^
            *                p2
            *              ^
            *             new p2
            * str:.....bdcf...
            *             ^
            *             p1
            *            ^
            *          new p1
            * 如上示意图所示，在比到e和c处不想等
            * p2返回到mark处，
            * p1需要返回到下一个位置。
            * 因为*前已经获得匹配，所以mark打标之前的不需要再比较
            */
            p1 -= p2 - mark - 1;
            p2 = mark;
            continue;
        }

        // 此处处理相等的情况
        p1++;
        p2++;
    }

    if (p2 == len2)
    {
        if (p1 == len1)
        {
            // 两个字符串都结束了，说明模式匹配成功
            return 0;
        }

        if (pattern[p2 - 1] == '*')
        {
            // str还没有结束，但pattern的最后一个字符是*，所以匹配成功
            return 0;
        }
    }

    while (p2 < len2)
    {
        // pattern多出的字符只要有一个不是*，匹配失败
        if (pattern[p2] != '*')
        {
            return -1;
        }

        p2++;
    }

    return 0;
}

std::string GetAbsolutePath(const char* path, const char* cur_work_dir)
{
    std::string absolute_path;

    if (path[0] != '/')
    {
        absolute_path.append(cur_work_dir);
        absolute_path.append("/");
        absolute_path.append(path);
    }
    else
    {
        absolute_path.append(path);
    }

    return absolute_path;

}
