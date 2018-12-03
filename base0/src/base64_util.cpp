#include "base64_util.h"
#include <string.h>
#include "openssl/pem.h"

int Base64Encode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                 int with_newline)
{
    BIO* b64 = BIO_new(BIO_f_base64());
    if (NULL == b64)
    {
        return -1;
    }

    if (!with_newline)
    {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }

    BIO* bmem = BIO_new(BIO_s_mem());
    if (NULL == bmem)
    {
        BIO_free_all(b64);
        return -2;
    }

    b64 = BIO_push(b64, bmem);

    BIO_write(b64, in_data, in_data_len); // encode
    //BIO_ctrl(b64, BIO_CTRL_FLUSH, 0, NULL);
    (void) BIO_flush(b64);

    BUF_MEM* bptr;
    BIO_get_mem_ptr(b64, &bptr);

    unsigned char* out_data_buf = new unsigned char[bptr->length + 1];
    if (NULL == out_data_buf)
    {
        BIO_free_all(b64);
        return -3;
    }

    memcpy(out_data_buf, bptr->data, bptr->length);
    out_data_buf[bptr->length] = '\0';

    *out_data = out_data_buf;
    out_data_len = (int) bptr->length;

    BIO_free_all(b64);
    return 0;
}

int Base64Decode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                 int with_newline)
{
    const size_t max_data_byte_count = (in_data_len * 6 + 7) / 8;

    unsigned char* out_data_buf = new unsigned char[max_data_byte_count + 1];
    if (NULL == out_data_buf)
    {
        return -1;
    }

    BIO* b64 = BIO_new(BIO_f_base64());
    if (NULL == b64)
    {
        delete[] out_data_buf;
        out_data_buf = NULL;
        return -2;
    }

    if (!with_newline)
    {
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    }

    BIO* bmem = BIO_new_mem_buf((void*) in_data, in_data_len);
    if (NULL == bmem)
    {
        BIO_free_all(b64);
        delete[] out_data_buf;
        out_data_buf = NULL;
        return -3;
    }

    b64 = BIO_push(b64, bmem);
    const size_t real_data_byte_count = BIO_read(b64, out_data_buf, max_data_byte_count);

    *out_data = out_data_buf;
    out_data_len = real_data_byte_count;

    BIO_free_all(b64);
    return 0;
}

void Base64Release(unsigned char** out_data)
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
