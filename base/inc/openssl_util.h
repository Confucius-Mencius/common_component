#ifndef BASE_INC_OPENSSL_UTIL_H_
#define BASE_INC_OPENSSL_UTIL_H_

#include <stddef.h>

int Base64Encode(unsigned char* out, const unsigned char* in, size_t len);

#endif // BASE_INC_OPENSSL_UTIL_H_
