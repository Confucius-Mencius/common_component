#include "md5_util.h"
#include "str_util.h"

int CalcMD5(char* buf, size_t buf_size, const unsigned char* data, size_t len)
{
    if (NULL == buf || buf_size <= MD5_STR_LEN || NULL == data || len < 1)
    {
        return -1;
    }

    unsigned char md[MD5_DIGEST_LENGTH] = "";

    int ret = CalcMD5Digest(md, sizeof(md), data, len);
    if (ret != 0)
    {
        return ret;
    }

    int offset = 0;
    int n;

    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        n = StrPrintf(buf + offset, buf_size - offset, "%02x", md[i]);
        offset += n;
    }

    return offset;
}

int CalcFileMD5(char* buf, size_t buf_size, const char* file_path)
{
    if (NULL == buf || buf_size <= MD5_STR_LEN || NULL == file_path)
    {
        return -1;
    }

    unsigned char md[MD5_DIGEST_LENGTH] = "";

    int ret = CalcFileMD5Digest(md, sizeof(md), file_path);
    if (ret != 0)
    {
        return ret;
    }

    int offset = 0;
    int n;

    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
    {
        n = StrPrintf(buf + offset, buf_size - offset, "%02x", md[i]);
        offset += n;
    }

    return offset;
}

int CalcMD5Digest(unsigned char* md, size_t buf_size, const unsigned char* data, size_t len)
{
    if (NULL == md || buf_size < MD5_DIGEST_LENGTH || NULL == data || len < 1)
    {
        return -1;
    }

    MD5(data, len, md);
    return 0;
}

int CalcFileMD5Digest(unsigned char* md, size_t buf_size, const char* file_path)
{
    if (NULL == md || buf_size < MD5_DIGEST_LENGTH || NULL == file_path)
    {
        return -1;
    }

    FILE* fp = fopen(file_path, "r");
    if (NULL == fp)
    {
        return -2;
    }

    MD5_CTX c;
    MD5_Init(&c);

    char tmp[4096] = "";

    while (0 == feof(fp))
    {
        size_t n = fread(tmp, 1, 4096, fp);
        if (n > 0)
        {
            MD5_Update(&c, tmp, n);
        }
    }

    fclose(fp);

    MD5_Final(md, &c);
    return 0;
}
