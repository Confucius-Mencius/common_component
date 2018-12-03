#include "rc4_util.h"
#include "openssl/rc4.h"

int RC4Encode(unsigned char** out_data, const unsigned char* in_data, size_t in_data_len, const unsigned char* passwd,
              int passwd_len)
{
    RC4_KEY rc4_key;
    RC4_set_key(&rc4_key, passwd_len, passwd);

    unsigned char* out_data_buf = new unsigned char[in_data_len + 1];
    if (NULL == out_data_buf)
    {
        return -1;
    }

    RC4(&rc4_key, in_data_len, in_data, out_data_buf);
    *out_data = out_data_buf;

    return 0;
}

int RC4Decode(unsigned char** out_data, const unsigned char* in_data, size_t in_data_len, const unsigned char* passwd,
              int passwd_len)
{
    RC4_KEY rc4Key;
    RC4_set_key(&rc4Key, passwd_len, passwd);

    unsigned char* out_data_buf = new unsigned char[in_data_len + 1];
    if (NULL == out_data_buf)
    {
        return -1;
    }

    RC4(&rc4Key, in_data_len, in_data, out_data_buf);
    *out_data = out_data_buf;

    return 0;
}

void RC4Release(unsigned char** out_data)
{
    if (NULL == out_data)
    {
        return;
    }

    if ((*out_data) != NULL)
    {
        delete[] (*out_data);
        (*out_data) = NULL;
    }
}
