#include "sha_util.h"
#include "openssl/crypto.h"
#include "str_util.h"

int CalcSHA1(char* buf, size_t buf_size, const unsigned char* data, size_t len)
{
    if (NULL == buf || buf_size <= SHA1_STR_LEN || NULL == data || len < 1)
    {
        return -1;
    }

    unsigned char md[SHA_DIGEST_LENGTH];

    int ret = CalcSHA1Digest(md, sizeof(md), data, len);
    if (ret != 0)
    {
        return ret;
    }

    int offset = 0;
    int n;

    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
    {
        n = StrPrintf(buf + offset, buf_size - offset, "%02x", md[i]);
        offset += n;
    }

    return offset;
}

int CalcFileSHA1(char* buf, size_t buf_size, const char* file_path)
{
    if (NULL == buf || buf_size <= SHA1_STR_LEN || NULL == file_path)
    {
        return -1;
    }

    unsigned char md[SHA_DIGEST_LENGTH];

    int ret = CalcFileSHA1Digest(md, sizeof(md), file_path);
    if (ret != 0)
    {
        return ret;
    }

    int offset = 0;
    int n;

    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
    {
        n = StrPrintf(buf + offset, buf_size - offset, "%02x", md[i]);
        offset += n;
    }

    return offset;
}

int CalcSHA1Digest(unsigned char* md, size_t buf_size, const unsigned char* data, size_t len)
{
    if (NULL == md || buf_size < SHA_DIGEST_LENGTH || NULL == data || len < 1)
    {
        return -1;
    }

    SHA1(data, len, md);
    return 0;
}

int CalcFileSHA1Digest(unsigned char* md, size_t buf_size, const char* file_path)
{
    if (NULL == md || buf_size < SHA_DIGEST_LENGTH || NULL == file_path)
    {
        return -1;
    }

    FILE* fp = fopen(file_path, "r");
    if (NULL == fp)
    {
        return -2;
    }

    SHA_CTX c;
    SHA1_Init(&c);

    char tmp[4096] = "";

    while (0 == feof(fp))
    {
        size_t n = fread(tmp, 1, 4096, fp);
        if (n > 0)
        {
            SHA1_Update(&c, tmp, n);
        }
    }

    fclose(fp);

    SHA1_Final(md, &c);
    OPENSSL_cleanse(&c, sizeof(c));

    return 0;
}

int CalcSHA1256(char* buf, size_t buf_size, const unsigned char* data, size_t len)
{
    if (NULL == buf || buf_size <= SHA256_STR_LEN || NULL == data || len < 1)
    {
        return -1;
    }

    unsigned char md[SHA256_DIGEST_LENGTH];

    int ret = CalcSHA256Digest(md, sizeof(md), data, len);
    if (ret != 0)
    {
        return ret;
    }

    int offset = 0;
    int n;

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        n = StrPrintf(buf + offset, buf_size - offset, "%02x", md[i]);
        offset += n;
    }

    return offset;
}

int CalcFileSHA256(char* buf, size_t buf_size, const char* file_path)
{
    if (NULL == buf || buf_size <= SHA256_STR_LEN || NULL == file_path)
    {
        return -1;
    }

    unsigned char md[SHA256_DIGEST_LENGTH];

    int ret = CalcFileSHA256Digest(md, sizeof(md), file_path);
    if (ret != 0)
    {
        return ret;
    }

    int offset = 0;
    int n;

    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
    {
        n = StrPrintf(buf + offset, buf_size - offset, "%02x", md[i]);
        offset += n;
    }

    return offset;
}

int CalcSHA256Digest(unsigned char* md, size_t buf_size, const unsigned char* data, size_t len)
{
    if (NULL == md || buf_size < SHA256_DIGEST_LENGTH || NULL == data || len < 1)
    {
        return -1;
    }

    SHA256(data, len, md);
    return 0;
}

int CalcFileSHA256Digest(unsigned char* md, size_t buf_size, const char* file_path)
{
    if (NULL == md || buf_size < SHA256_DIGEST_LENGTH || NULL == file_path)
    {
        return -1;
    }

    FILE* fp = fopen(file_path, "r");
    if (NULL == fp)
    {
        return -2;
    }

    SHA256_CTX c;
    SHA256_Init(&c);

    char tmp[4096] = "";

    while (0 == feof(fp))
    {
        size_t n = fread(tmp, 1, 4096, fp);
        if (n > 0)
        {
            SHA256_Update(&c, tmp, n);
        }
    }

    fclose(fp);

    SHA256_Final(md, &c);
    OPENSSL_cleanse(&c, sizeof(c));

    return 0;
}
