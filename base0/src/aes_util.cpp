#include "aes_util.h"
#include <string.h>

// 加密块大小（分组）必须为128位（16字节），如果不是，则要补齐
static size_t CalcDataBufByteCount(size_t in_data_len)
{
    if (0 == (in_data_len % AES_BLOCK_SIZE))
    {
        return in_data_len;
    }
    else
    {
        return ((in_data_len / AES_BLOCK_SIZE) + 1) * AES_BLOCK_SIZE;
    }
}

int AESEncode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
              const unsigned char* passwd, AESPasswdLen passwd_bit_count)
{
    if (NULL == out_data || NULL == in_data || in_data_len < 1 || NULL == passwd)
    {
        return -1;
    }

    if (passwd_bit_count != AES_PASSWD_BIT_128 && passwd_bit_count != AES_PASSWD_BIT_192
        && passwd_bit_count != AES_PASSWD_BIT_256)
    {
        return -1;
    }

    AES_KEY aes_key;
    if (AES_set_encrypt_key(passwd, passwd_bit_count, &aes_key) < 0)
    {
        return -1;
    }

    const size_t data_buf_byte_count = CalcDataBufByteCount(in_data_len);

    unsigned char* in_data_buf = new unsigned char[data_buf_byte_count];
    if (NULL == in_data_buf)
    {
        return -1;
    }

    memcpy(in_data_buf, in_data, in_data_len);

    unsigned char* out_data_buf = new unsigned char[data_buf_byte_count];
    if (NULL == out_data)
    {
        delete[] in_data_buf;
        in_data_buf = NULL;
        return -1;
    }

    unsigned char ivec[AES_BLOCK_SIZE];
    memset(ivec, 0, sizeof(ivec));

    AES_cbc_encrypt(in_data_buf, out_data_buf, data_buf_byte_count, &aes_key, ivec, AES_ENCRYPT);

    *out_data = out_data_buf;
    out_data_len = data_buf_byte_count;

    delete[] in_data_buf;
    in_data_buf = NULL;

    return 0;
}

int AESDecode(unsigned char* out_data, const unsigned char* in_data, size_t in_data_len, const unsigned char* passwd,
              AESPasswdLen passwd_len)
{
    if (NULL == out_data || NULL == in_data || in_data_len < 1 || NULL == passwd)
    {
        return -1;
    }

    if (passwd_len != AES_PASSWD_BIT_128 && passwd_len != AES_PASSWD_BIT_192
        && passwd_len != AES_PASSWD_BIT_256)
    {
        return -1;
    }

    AES_KEY aes_key;
    if (AES_set_decrypt_key(passwd, passwd_len, &aes_key) < 0)
    {
        return -1;
    }

    unsigned char ivec[AES_BLOCK_SIZE];
    memset(ivec, 0, sizeof(ivec));

    AES_cbc_encrypt(in_data, out_data, in_data_len, &aes_key, ivec, AES_DECRYPT);
    return 0;
}

void AESRelease(unsigned char** out_data)
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
