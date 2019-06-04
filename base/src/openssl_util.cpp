#include "openssl_util.h"
#include <string.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

int Base64Encode(unsigned char* out, const unsigned char* in, size_t len)
{
    BIO* b64, *bio;
    BUF_MEM* bptr = NULL;
    size_t size = 0;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_write(bio, in, len);
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bptr);
    memcpy(out, bptr->data, bptr->length);
    out[bptr->length] = '\0';
    size = bptr->length;

    BIO_free_all(bio);
    return size;
}
