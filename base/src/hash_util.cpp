#include "hash_util.h"

unsigned long HashPJW(const char* data, size_t len)
{
    const char* end = data + len;
    unsigned long h = 0;
    unsigned long g;

    while (data < end)
    {
        h = (h << 4) + *data++;

        if ((g = (h & 0xF0000000)))
        {
            h = h ^ (g >> 24);
            h = h ^ g;
        }
    }

    return h;
}
