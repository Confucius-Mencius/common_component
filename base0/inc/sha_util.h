#ifndef BASE_INC_SHA_UTIL_H_
#define BASE_INC_SHA_UTIL_H_

#include "openssl/sha.h"

/**
 * @breif SHA1字符串的长度
 * @hideinitializer
 */
#define SHA1_STR_LEN (SHA_DIGEST_LENGTH * 2)
#define SHA256_STR_LEN (SHA256_DIGEST_LENGTH * 2)

////////////////////////////////////////////////////////////////////////////////
// sha1(160)
int CalcSHA1(char buf[SHA1_STR_LEN + 1], size_t buf_size, const unsigned char* data, size_t len);
int CalcFileSHA1(char buf[SHA1_STR_LEN + 1], size_t buf_size, const char* file_path);

int CalcSHA1Digest(unsigned char md[SHA_DIGEST_LENGTH], size_t buf_size, const unsigned char* data, size_t len);
int CalcFileSHA1Digest(unsigned char md[SHA_DIGEST_LENGTH], size_t buf_size, const char* file_path);

////////////////////////////////////////////////////////////////////////////////
// sha256
int CalcSHA1256(char buf[SHA256_STR_LEN + 1], size_t buf_size, const unsigned char* data, size_t len);
int CalcFileSHA256(char buf[SHA256_STR_LEN + 1], size_t buf_size, const char* file_path);

int CalcSHA256Digest(unsigned char md[SHA256_DIGEST_LENGTH], size_t buf_size, const unsigned char* data, size_t len);
int CalcFileSHA256Digest(unsigned char md[SHA256_DIGEST_LENGTH], size_t buf_size, const char* file_path);

#endif // BASE_INC_SHA_UTIL_H_
