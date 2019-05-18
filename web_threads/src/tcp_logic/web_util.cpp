#include "web_util.h"
#include <ctype.h>
#include <stdlib.h>

static unsigned char hexchars[] = "0123456789abcdef";
/**
 * 16进制数转换成10进制数
 * 如：0xE4=14*16+4=228
 */
static int _htoi(char* s)
{
    int value;
    int c;

    c = ((unsigned char*) s)[0];
    if (isupper(c))
    {
        c = tolower(c);
    }
    value = (c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10) * 16;

    c = ((unsigned char*) s)[1];
    if (isupper(c))
    {
        c = tolower(c);
    }
    value += c >= '0' && c <= '9' ? c - '0' : c - 'a' + 10;

    return (value);
}

char* URLEncode(int& encoded_len, const char* s, size_t len)
{
    register unsigned char c;
    unsigned char* to, *start;
    const unsigned char* from, *end;

    from = (unsigned char*) s;
    end  = (unsigned char*) s + len;
    start = to = (unsigned char*) calloc(1, 3 * len + 1);

    while (from < end)
    {
        c = *from++;

        if (c == ' ')
        {
            *to++ = '+';
        }
        else if ((c < '0' && c != '.' && c != '-' && c != '*')
                 || (c < 'A' && c > '9')
                 || (c > 'Z' && c < 'a' && c != '_')
                 || (c > 'z')) // 在标准实现中，~也可以排除。（这里没有）
        {
            to[0] = '%';
            to[1] = hexchars[c >> 4]; // 将二进制转换成十六进制表示
            to[2] = hexchars[c & 15]; // 将二进制转换成十六进制表示
            to += 3;
        }
        else
        {
            *to++ = c;
        }
    }

    *to = '\0';
    encoded_len = to - start;

    return (char*) start;
}

size_t URLDecode(char* s, size_t len)
{
    char* dest = s;
    char* data = s;

    while (len--)
    {
        if (*data == '+')
        {
            *dest = ' ';
        }
        else if (*data == '%' && len >= 2 && isxdigit((int) * (data + 1)) && isxdigit((int) * (data + 2)))
        {
            *dest = (char) _htoi(data + 1);
            data += 2;
            len -= 2;
        }
        else
        {
            *dest = *data;
        }

        data++;
        dest++;
    }

    *dest = '\0';
    return (dest - s);
}

void URLFree(char* s)
{
    if (s != nullptr)
    {
        free(s);
    }
}
