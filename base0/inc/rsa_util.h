#ifndef BASE_INC_RSA_UTIL_H_
#define BASE_INC_RSA_UTIL_H_

#include "openssl/rsa.h"
#include "common_define.h"
#include "last_err_msg.h"

/**
 * 在linux下，使用如下命令，生成RSA加解密时使用的public和private的key：
 * openssl genrsa -out rsa_private_key.pem 1024  // 生成私钥，1024是生成私钥的bit位数，编码是PKCS#1格式
 * openssl rsa -in rsa_private_key.pem -out rsa_public_key.pem -pubout  // 生成对应的公钥
 */

/**
 * @brief rsa加解密
 * @attention RSA的API中当使用参数RSA_PKCS1_PADDING时，明文长度不能大于(秘钥bit位数/8 - 11)，需要分解成符合要求的块进行加密
 *  加解密会改变数据长度
 *  公钥加密则用私钥来解密，私钥加密则用公钥来解密
 */
class RSAUtil
{
public:
    enum Padding
    {
        PADDING_NO = RSA_NO_PADDING, /**< 不安全，不建议使用 */
        PADDING_PKCS1 = RSA_PKCS1_PADDING, /**< 目前使用较广泛 */
        PADDING_PKCS1_OAEP = RSA_PKCS1_OAEP_PADDING /**< 推荐新的应用使用 */
    };

    static int GetPaddingSize(Padding padding);

public:
    RSAUtil();
    ~RSAUtil();

    /**
     * @brief 获取最近一次的错误信息
     */
    const char* GetLastErrMsg() const;

    /**
     * @brief 初始化秘钥文件
     * @return 返回0表示成功,否则为错误码
     */
    int Initialize(const char* pub_key_file_path, const char* pri_key_file_path);

    /**
     * @brief 清理资源
     */
    void Finalize();

    /**
     * @brief RSA公钥加密
     * @return 返回0表示成功,否则为错误码
     * @attention 使用完毕需要调用CleanUp释放内存
     */
    int PubEncode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                  Padding padding);

    /**
     * @brief RSA私钥解密
     * @return 返回0表示成功,否则为错误码
     * @attention 使用完毕需要调用CleanUp释放内存
     */
    int PriDecode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                  Padding padding);

    /**
     * @brief RSA私钥加密
     * @return 返回0表示成功,否则为错误码
     * @attention
     *  使用完毕需要调用CleanUp释放内存
     *  private key encryption supports only these paddings: RSA_PKCS1_PADDING and RSA_NO_PADDING.
     */
    int PriEncode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                  Padding padding);

    /**
     * @brief RSA公钥解密
     * @return 返回0表示成功,否则为错误码
     * @attention 使用完毕需要调用CleanUp释放内存
     */
    int PubDecode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                  Padding padding);

    /**
     * @brief 清理Encode/Decode的内存
     * @details Encode/Decode获得的out_data在使用完毕后需要调用该接口清理内存
     */
    void Release();

private:
    RSA* CreateRSA(const char* file_path, int is_public);
    void SetOpensslLastErrMsg();

private:
    LastErrMsg last_err_msg_;
    RSA* rsa_pub_;
    int rsa_pub_size_;
    unsigned char* pub_buf_;
    RSA* rsa_pri_;
    int rsa_pri_size_;
    unsigned char* pri_buf_;
    unsigned char* out_data_buf_;
};

#endif // BASE_INC_RSA_UTIL_H_
