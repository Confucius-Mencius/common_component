#include "hex_dump.h"
#include "str_util.h"

int HexDump(char* buf, size_t buf_size, const void* data, size_t len)
{
    if (NULL == data || len <= 0 || NULL == buf || buf_size <= 0)
    {
        return -1;
    }

    const int BYTES_PER_LINE = 0x10; // 每行输出16个字节
    const unsigned char* const p = (const unsigned char*) data;
    int offset = 0;

    // 表头
    char head[] = "        ";
    int n = StrPrintf(buf + offset, buf_size - offset, "%s", head);
    offset += n;

    for (int i = 0; i < BYTES_PER_LINE; ++i)
    {
        if (buf_size - offset <= 1)
        {
            return -2;
        }

        n = StrPrintf(buf + offset, buf_size - offset, "%3x", i);
        offset += n;
    }

    for (size_t i = 0; i < len; ++i)
    {
        if (buf_size - offset <= 1)
        {
            return -3;
        }

        // 行号
        if (0 == (i % BYTES_PER_LINE))
        {
            n = StrPrintf(buf + offset, buf_size - offset, "\n%08x:", i / BYTES_PER_LINE + 1);
            offset += n;
        }

        // 二进制数据
        n = StrPrintf(buf + offset, buf_size - offset, " %02x", (int) p[i]);
        offset += n;
    }

    return offset;
}
