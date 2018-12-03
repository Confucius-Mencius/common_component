#ifndef BASE_INC_AES_UTIL_H_
#define BASE_INC_AES_UTIL_H_

#include "openssl/aes.h"

// AES有几种扩展算法，其中ecb和cbc需要填充，即加密后长度可能会不一样，cfb和ofb不需要填充，密文长度与明文长度一样

/**
 * @brief AES秘钥长度
 * @attention 密钥长度可以选择128位、192位、256位,所以密码长度必须是16,24,32字节
 */
enum AESPasswdLen
{
    AES_PASSWD_BIT_128 = 128,
    AES_PASSWD_BIT_192 = 192,
    AES_PASSWD_BIT_256 = 256
};

/**
 * @brief AES加密
 * @param out_data
 * @param out_data_len
 * @param in_data
 * @param in_data_len
 * @param passwd
 * @param passwd_bit_count
 * @return
 * @attention 使用完毕需要调用AESRelease释放out_data内存
 */
int AESEncode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
              const unsigned char* passwd, AESPasswdLen passwd_bit_count);

/**
 * @brief AES解密
 * @param out_data
 * @param in_data
 * @param in_data_len
 * @param passwd
 * @param passwd_len
 * @return
 */
int AESDecode(unsigned char* out_data, const unsigned char* in_data, size_t in_data_len,
              const unsigned char* passwd, AESPasswdLen passwd_len);

/**
 * @brief 清理Encode的内存
 * @details Encode获得的out_data在使用完毕后需要调用该接口清理内存
 * @param out_data
 */
void AESRelease(unsigned char** out_data);

#endif // BASE_INC_AES_UTIL_H_
