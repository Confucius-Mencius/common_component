/**
 * @file simple_str.h
 * @brief 自定义的string类
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef BASE_INC_SIMPLE_STR_H_
#define BASE_INC_SIMPLE_STR_H_

/**
 * @defgroup Module_Base 基础库
 * @{
 */

/**
 * @defgroup Module_SimpleStr SimpleStr
 * @{
 */

#include <istream>
#include <ostream>
#include "str_util.h"

template<int buf_size>
class SimpleStr
{
public:
    SimpleStr()
    {
        buf_[0] = '\0';
        buf_[buf_size - 1] = '\0';
    }

    SimpleStr(const SimpleStr& rhs)
    {
        if (&rhs != this)
        {
            StrCpy(buf_, sizeof(buf_), rhs.Value());
            buf_[buf_size - 1] = '\0';
        }
    }

    SimpleStr(const char* s)
    {
        if (NULL == s)
        {
            buf_[0] = '\0';
            buf_[buf_size - 1] = '\0';
        }
        else
        {
            StrCpy(buf_, sizeof(buf_), s);
            buf_[buf_size - 1] = '\0';
        }
    }

    SimpleStr(char c, int n)
    {
        memset(buf_, 0, sizeof(buf_));

        for (int i = 0; i < n && n < buf_size; ++i)
        {
            buf_[i] = c;
        }

        buf_[buf_size - 1] = '\0';
    }

    ~SimpleStr()
    {
    }

    int Len() const
    {
        return StrLen(buf_, sizeof(buf_));
    }

    const char* Value() const
    {
        return buf_;
    }

    SimpleStr& operator=(const SimpleStr& rhs)
    {
        if (&rhs != this)
        {
            StrCpy(buf_, sizeof(buf_), rhs.Value());
        }

        return *this;
    }

    SimpleStr& operator=(const char* s)
    {
        if (NULL == s)
        {
            buf_[0] = '\0';
        }
        else
        {
            StrCpy(buf_, sizeof(buf_), s);
        }

        return *this;
    }

    SimpleStr& operator+(const SimpleStr& rhs)
    {
        StrCat(buf_, sizeof(buf_) - Len(), rhs.Value());
        return *this;
    }

    SimpleStr& operator+(const char* s)
    {
        if (s != NULL)
        {
            StrCat(buf_, sizeof(buf_) - Len(), s);
        }

        return *this;
    }

    SimpleStr& operator+=(const SimpleStr& str)
    {
        return this->operator+(str);
    }

    SimpleStr& operator+=(const char* s)
    {
        return this->operator+(s);
    }

    char& operator[](int idx)
    {
        if (idx < 0 || idx >= Len())
        {
            return buf_[buf_size - 1];
        }

        return buf_[idx];
    }

    const char& operator[](int idx) const
    {
        if (idx < 0 || idx >= Len())
        {
            return buf_[buf_size - 1];
        }

        return buf_[idx];
    }

    friend std::ostream& operator<<(std::ostream& os, const SimpleStr& instance)
    {
        os << instance.Value();
        return os;
    }

    friend std::istream& operator>>(std::istream& is, SimpleStr& instance)
    {
        if (is)
        {
            char c;
            int n = 0;

            while (is.get(c) && !isspace(c) && n < buf_size)
            {
                instance.buf_[n++] = c;
            }
        }

        return is;
    }

private:
    char buf_[buf_size];
};

/** @} Module_SimpleStr */
/** @} Module_Base */

#endif // BASE_INC_SIMPLE_STR_H_
