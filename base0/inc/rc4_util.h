#ifndef BASE_INC_RC4_UTIL_H_
#define BASE_INC_RC4_UTIL_H_

#include <stddef.h>

// RC4加密后，密文和明文的长度一样

/**
 * @brief RC4加密
 * @param out_data
 * @param in_data
 * @param in_data_len
 * @param passwd
 * @param passwd_len
 * @return
 * @attention 使用完毕需要调用RC4Release释放out_data内存
 */
int RC4Encode(unsigned char** out_data, const unsigned char* in_data, size_t in_data_len, const unsigned char* passwd,
              int passwd_len);

/**
 *
 * @param out_data
 * @param in_data
 * @param in_data_len
 * @param passwd
 * @param passwd_len
 * @return
 * @attention 使用完毕需要调用RC4Release释放out_data内存
 */
int RC4Decode(unsigned char** out_data, const unsigned char* in_data, size_t in_data_len, const unsigned char* passwd,
              int passwd_len);

/**
 * @brief 清理Encode/Decode的内存
 * @details Encode/Decode获得的out_data在使用完毕后需要调用该接口清理内存
 * @param out_data
 */
void RC4Release(unsigned char** out_data);

#endif // BASE_INC_RC4_UTIL_H_
