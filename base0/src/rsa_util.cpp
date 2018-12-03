#include "rsa_util.h"
#include "openssl/pem.h"
#include "openssl/err.h"
#include "file_util.h"

int RSAUtil::GetPaddingSize(RSAUtil::Padding padding)
{
    int size = -1;

    switch (padding)
    {
        case PADDING_NO:
        {
            size = 0;
        }
            break;

        case PADDING_PKCS1:
        {
            size = RSA_PKCS1_PADDING_SIZE;
        }
            break;

        case PADDING_PKCS1_OAEP:
        {
            size = 2 * SHA_DIGEST_LENGTH + 2;
        }

        default:
        {
        }
            break;
    }

    return size;
}

RSAUtil::RSAUtil() : last_err_msg_()
{
    rsa_pub_ = NULL;
    rsa_pub_size_ = 0;
    pub_buf_ = NULL;
    rsa_pri_ = NULL;
    rsa_pri_size_ = 0;
    pri_buf_ = NULL;
    out_data_buf_ = NULL;
}

RSAUtil::~RSAUtil()
{
}

const char* RSAUtil::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

int RSAUtil::Initialize(const char* pub_key_file_path, const char* pri_key_file_path)
{
    if (NULL == pub_key_file_path || NULL == pri_key_file_path)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "param is null");
        return -1;
    }

    if (!FileExist(pub_key_file_path) || !FileExist(pri_key_file_path))
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "key file not exist");
        return -1;
    }

    rsa_pub_ = CreateRSA(pub_key_file_path, 1);
    if (NULL == rsa_pub_)
    {
        return -1;
    }

    rsa_pub_size_ = RSA_size(rsa_pub_);

    pub_buf_ = new unsigned char[rsa_pub_size_];
    if (NULL == pub_buf_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory");
        return -1;
    }

    rsa_pri_ = CreateRSA(pri_key_file_path, 0);
    if (NULL == rsa_pri_)
    {
        return -1;
    }

    rsa_pri_size_ = RSA_size(rsa_pri_);

    pri_buf_ = new unsigned char[rsa_pri_size_];
    if (NULL == pri_buf_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory");
        return -1;
    }

    OpenSSL_add_all_algorithms();
    OpenSSL_add_all_ciphers();
    OpenSSL_add_all_digests();

    return 0;
}

void RSAUtil::Finalize()
{
    if (pub_buf_ != NULL)
    {
        delete[] pub_buf_;
        pub_buf_ = NULL;
    }

    if (rsa_pub_ != NULL)
    {
        RSA_free(rsa_pub_);
        rsa_pub_ = NULL;
    }

    if (pri_buf_ != NULL)
    {
        delete[] pri_buf_;
        pri_buf_ = NULL;
    }

    if (rsa_pri_ != NULL)
    {
        RSA_free(rsa_pri_);
        rsa_pri_ = NULL;
    }
}

int RSAUtil::PubEncode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                       Padding padding)
{

    if (padding != PADDING_PKCS1 && padding != PADDING_PKCS1_OAEP && padding != PADDING_NO)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "not supported padding: " << padding);
        return -1;
    }

    RSA* rsa = rsa_pub_;
    const int rsa_size = rsa_pub_size_;
    unsigned char* buf = pub_buf_;

    // 跟AES一样，RSA也是一个块加密算法(block cipher algorithm)，即总是在一个固定长度的块上进行操作
    const int block_len = (rsa_size - GetPaddingSize(padding));
    const int loop_max = in_data_len / block_len;
    size_t in_data_len_left = in_data_len;
    int offset = 0;
    std::string result = "";
    int i = 0;
    int ret;

    for (i = 0; i < loop_max; ++i)
    {
        offset = i * block_len;
        memset(buf, 0, sizeof(rsa_size));

        ret = RSA_public_encrypt(block_len, in_data + offset, buf, rsa, padding);
        if (ret < 0)
        {
            SetOpensslLastErrMsg();
            CRYPTO_cleanup_all_ex_data();
            return -1;
        }

        result.append((const char*) buf, ret);
        out_data_len += rsa_size;
        in_data_len_left -= block_len;
    }

    if (in_data_len_left > 0)
    {
        offset = i * block_len;
        memset(buf, 0, sizeof(rsa_size + 1));

        ret = RSA_public_encrypt(in_data_len_left, in_data + offset, buf, rsa, padding);
        if (ret < 0)
        {
            SetOpensslLastErrMsg();
            CRYPTO_cleanup_all_ex_data();
            return -1;
        }

        result.append((const char*) buf, ret);
        out_data_len += rsa_size;
    }

    CRYPTO_cleanup_all_ex_data();

    out_data_buf_ = new unsigned char[result.size() + 1];
    if (NULL == out_data_buf_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory");
        return -1;
    }

    memcpy(out_data_buf_, result.data(), result.size());
    out_data_buf_[result.size()] = '\0';

    *out_data = out_data_buf_;
    out_data_len = result.size();

    return 0;
}

int RSAUtil::PriDecode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                       Padding padding)
{
    if (padding != PADDING_PKCS1 && padding != PADDING_PKCS1_OAEP && padding != PADDING_NO)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "not supported padding: " << padding);
        return -1;
    }

    RSA* rsa = rsa_pri_;
    const int rsa_size = rsa_pri_size_;
    unsigned char* buf = pri_buf_;

    const int loop_max = in_data_len / rsa_size;
    size_t in_data_len_left = in_data_len;
    int offset = 0;
    std::string result = "";
    int i = 0;
    int ret;

    for (i = 0; i < loop_max; ++i)
    {
        offset = i * rsa_size;
        memset(buf, 0, sizeof(rsa_size));

        ret = RSA_private_decrypt(rsa_size, in_data + offset, buf, rsa, padding);
        if (ret < 0)
        {
            SetOpensslLastErrMsg();
            CRYPTO_cleanup_all_ex_data();
            return -1;
        }

        result.append((const char*) buf, ret);
        out_data_len += ret;
        in_data_len_left -= rsa_size;
    }

    CRYPTO_cleanup_all_ex_data();

    out_data_buf_ = new unsigned char[result.size() + 1];
    if (NULL == out_data_buf_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory");
        return -1;
    }

    memcpy(out_data_buf_, result.data(), result.size());
    out_data_buf_[result.size()] = '\0';

    *out_data = out_data_buf_;
    out_data_len = result.size();

    return 0;
}

int RSAUtil::PriEncode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                       Padding padding)
{
    if (padding != PADDING_PKCS1 && padding != PADDING_NO)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "invalid padding, private key encryption supports only these paddings: "
                                         "RSA_PKCS1_PADDING and RSA_NO_PADDING.");
        return -1;
    }

    RSA* rsa = rsa_pri_;
    const int rsa_size = rsa_pri_size_;
    unsigned char* buf = pri_buf_;

    const int block_len = (rsa_size - GetPaddingSize(padding));
    const int loop_max = in_data_len / block_len;
    size_t in_data_len_left = in_data_len;
    int offset = 0;
    std::string result = "";
    int i = 0;
    int ret;

    for (i = 0; i < loop_max; ++i)
    {
        offset = i * block_len;
        memset(buf, 0, sizeof(rsa_size));

        ret = RSA_private_encrypt(block_len, in_data + offset, buf, rsa, padding);
        if (ret < 0)
        {
            SetOpensslLastErrMsg();
            CRYPTO_cleanup_all_ex_data();
            return -1;
        }

        result.append((const char*) buf, ret);
        out_data_len += rsa_size;
        in_data_len_left -= block_len;
    }

    if (in_data_len_left > 0)
    {
        offset = i * block_len;
        memset(buf, 0, sizeof(rsa_size + 1));

        ret = RSA_private_encrypt(in_data_len_left, in_data + offset, buf, rsa, padding);
        if (ret < 0)
        {
            SetOpensslLastErrMsg();
            CRYPTO_cleanup_all_ex_data();
            return -1;
        }

        result.append((const char*) buf, ret);
        out_data_len += rsa_size;
    }

    CRYPTO_cleanup_all_ex_data();

    out_data_buf_ = new unsigned char[result.size() + 1];
    if (NULL == out_data_buf_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory");
        return -1;
    }

    memcpy(out_data_buf_, result.data(), result.size());
    out_data_buf_[result.size()] = '\0';

    *out_data = out_data_buf_;
    out_data_len = result.size();

    return 0;
}

int RSAUtil::PubDecode(unsigned char** out_data, size_t& out_data_len, const unsigned char* in_data, size_t in_data_len,
                       Padding padding)
{
    if (padding != PADDING_PKCS1 && padding != PADDING_NO)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "invalid padding, public key decryption supports only these paddings: "
                                         "RSA_PKCS1_PADDING and RSA_NO_PADDING.");
        return -1;
    }

    RSA* rsa = rsa_pub_;
    const int rsa_size = rsa_pub_size_;
    unsigned char* buf = pub_buf_;

    const int loop_max = in_data_len / rsa_size;
    size_t in_data_len_left = in_data_len;
    int offset = 0;
    std::string result = "";
    int i = 0;
    int ret;

    for (i = 0; i < loop_max; ++i)
    {
        offset = i * rsa_size;
        memset(buf, 0, sizeof(rsa_size));

        ret = RSA_public_decrypt(rsa_size, in_data + offset, buf, rsa, padding);
        if (ret < 0)
        {
            SetOpensslLastErrMsg();
            CRYPTO_cleanup_all_ex_data();
            return -1;
        }

        result.append((const char*) buf, ret);
        out_data_len += ret;
        in_data_len_left -= rsa_size;
    }

    CRYPTO_cleanup_all_ex_data();

    out_data_buf_ = new unsigned char[result.size() + 1];
    if (NULL == out_data_buf_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory");
        return -1;
    }

    memcpy(out_data_buf_, result.data(), result.size());
    out_data_buf_[result.size()] = '\0';

    *out_data = out_data_buf_;
    out_data_len = result.size();

    return 0;
}

void RSAUtil::Release()
{
    if (out_data_buf_ != NULL)
    {
        delete[] out_data_buf_;
        out_data_buf_ = NULL;
    }
}

RSA* RSAUtil::CreateRSA(const char* file_path, int is_public)
{
    FILE* fp = NULL;
    RSA* rsa = NULL;

    do
    {
        fp = fopen(file_path, "rb");
        if (NULL == fp)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "failed to open file " << file_path);
            break;
        }

        rsa = (is_public ? PEM_read_RSA_PUBKEY(fp, NULL, NULL, NULL) : PEM_read_RSAPrivateKey(fp, NULL, NULL, NULL));
        if (NULL == rsa)
        {
            SetOpensslLastErrMsg();
            break;
        }
    } while (0);

    if (fp != NULL)
    {
        fclose(fp);
        fp = NULL;
    }

    return rsa;
}

void RSAUtil::SetOpensslLastErrMsg()
{
    char err[1024] = "";
    ERR_load_crypto_strings();
    ERR_error_string(ERR_get_error(), err);
    SET_LAST_ERR_MSG(&last_err_msg_, err);
}
