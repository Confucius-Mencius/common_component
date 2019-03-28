#include "uuid_util.h"
#include "str_util.h"

UUID MakeUUID()
{
    UUID uuid;

#if (defined(__linux__))
    ::uuid_generate((unsigned char*) &uuid);
#elif (defined(_WIN32) || defined(_WIN64))
    ::CoCreateGuid(&uuid);
#endif

    return uuid;
}

char* UUID38(char* buf, size_t buf_size, const UUID& uuid)
{
    if (nullptr == buf || buf_size != (UUID_LEN_38 + 1))
    {
        return NULL;
    }

    StrPrintf(buf, buf_size,
              "{%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}", // 38个字符
              uuid.Data1, uuid.Data2, uuid.Data3, uuid.Data4[0], uuid.Data4[1],
              uuid.Data4[2], uuid.Data4[3], uuid.Data4[4], uuid.Data4[5],
              uuid.Data4[6], uuid.Data4[7]);

    return buf;
}

char* UUID32(char* buf, size_t buf_size, const UUID& uuid)
{
    if (nullptr == buf || buf_size != (UUID_LEN_32 + 1))
    {
        return NULL;
    }

    StrPrintf(buf, buf_size,
              "%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X", // 32个字符
              uuid.Data1, uuid.Data2, uuid.Data3, uuid.Data4[0], uuid.Data4[1],
              uuid.Data4[2], uuid.Data4[3], uuid.Data4[4], uuid.Data4[5],
              uuid.Data4[6], uuid.Data4[7]);

    return buf;
}
