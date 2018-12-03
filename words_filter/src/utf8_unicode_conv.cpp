#include "utf8_unicode_conv.h"
#include <errno.h>
#include <iconv.h>
#include <string.h>

UTF8UnicodeConv::UTF8UnicodeConv() : last_err_msg_()
{
}

UTF8UnicodeConv::~UTF8UnicodeConv()
{
}

int UTF8UnicodeConv::UTF82Unicode(wchar_t* dst, size_t dst_buf_size, const char* src, size_t src_bytes)
{
    iconv_t utf8_to_unicode_conv = iconv_open("WCHAR_T", "UTF-8");
    if ((iconv_t) -1 == utf8_to_unicode_conv)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "iconv_open failed, errno: " << errno << ", err: " << strerror(errno));
        return -1;
    }

    char* in = (char*) src;
    char* out = (char*) dst;

    size_t ret = iconv(utf8_to_unicode_conv, &in, &src_bytes, &out, &dst_buf_size);
    if ((size_t) -1 == ret)
    {
        // errno: 7, err: Argument list too long表示输出缓冲区太小
        SET_LAST_ERR_MSG(&last_err_msg_, "iconv failed, errno: " << errno << ", err: " << strerror(errno));
        return -1;
    }

    iconv_close(utf8_to_unicode_conv);
    return 0;
}

int UTF8UnicodeConv::Unicode2UTF8(char* dst, size_t dst_buf_size, const wchar_t* src, size_t src_bytes)
{
    iconv_t unicode_to_utf8_conv = iconv_open("UTF-8", "WCHAR_T");
    if ((iconv_t) -1 == unicode_to_utf8_conv)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "iconv_open, errno: " << errno << ", err: " << strerror(errno));
        return -1;
    }

    char* in = (char*) src;
    char* out = dst;

    size_t ret = iconv(unicode_to_utf8_conv, &in, &src_bytes, &out, &dst_buf_size);
    if ((size_t) -1 == ret)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "iconv failed, errno: " << errno << ", err: " << strerror(errno));
        return -1;
    }

    iconv_close(unicode_to_utf8_conv);
    return 0;
}
