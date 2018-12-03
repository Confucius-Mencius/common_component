#include "rsa_util_test.h"
#include "hex_dump.h"

/**
* openssl genrsa -out rsa_private_key.pem 1024  // 生成私钥，1024是生成私钥的bit位数，编码是PKCS#1格式
* openssl rsa -in rsa_private_key.pem -out rsa_public_key.pem -pubout  // 生成对应的公钥
*/
static const int PRI_KEY_BITS = 1024;

// 公钥加密私钥解密
static void Func1(RSAUtil& rsa_util, const unsigned char* in_data, size_t in_data_len, RSAUtil::Padding padding)
{
    char hex_dump_buf[4096] = "";

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), in_data, in_data_len);
    std::cout << hex_dump_buf << std::endl;

    unsigned char* out_data = NULL;
    size_t out_data_len = 0;

    if (rsa_util.PubEncode(&out_data, out_data_len, in_data, in_data_len, padding) != 0)
    {
        rsa_util.Release();
        FAIL() << rsa_util.GetLastErrMsg();
    }

    //  取出结果
    unsigned char* out_data_buf = new unsigned char[out_data_len + 1];
    memset(out_data_buf, 0, out_data_len + 1);
    memcpy(out_data_buf, out_data, out_data_len);

    // 释放内存
    rsa_util.Release();

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), out_data_buf, out_data_len);
    std::cout << hex_dump_buf << std::endl;

    unsigned char* origin_in_data = NULL;
    size_t origin_in_data_len = 0;

    if (rsa_util.PriDecode(&origin_in_data, origin_in_data_len, out_data_buf, out_data_len, padding) != 0)
    {
        rsa_util.Release();

        delete[] out_data_buf;
        out_data_buf = NULL;

        FAIL() << rsa_util.GetLastErrMsg();
    }

    std::cout << "out_data_len: " << out_data_len << ", origin_in_data_len: " << origin_in_data_len << std::endl;

    // 取出结果
    unsigned char* origin_in_data_buf = new unsigned char[origin_in_data_len + 1];
    memset(origin_in_data_buf, 0, origin_in_data_len + 1);
    memcpy(origin_in_data_buf, origin_in_data, origin_in_data_len);

    // 释放内存
    rsa_util.Release();

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), origin_in_data_buf, origin_in_data_len);
    std::cout << hex_dump_buf << std::endl;

    EXPECT_EQ(in_data_len, origin_in_data_len);
    EXPECT_STREQ((const char*) in_data, (const char*) origin_in_data_buf);

    delete[] origin_in_data_buf;
    origin_in_data_buf = NULL;

    delete[] out_data_buf;
    out_data_buf = NULL;
}

// 私钥加密公钥解密
static void Func2(RSAUtil& rsa_util, const unsigned char* in_data, size_t in_data_len, RSAUtil::Padding padding)
{
    char hex_dump_buf[4096] = "";

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), in_data, in_data_len);
    std::cout << hex_dump_buf << std::endl;

    unsigned char* out_data = NULL;
    size_t out_data_len = 0;

    if (rsa_util.PriEncode(&out_data, out_data_len, in_data, in_data_len, padding) != 0)
    {
        rsa_util.Release();
        FAIL() << rsa_util.GetLastErrMsg();
    }

    //  取出结果
    unsigned char* out_data_buf = new unsigned char[out_data_len + 1];
    memset(out_data_buf, 0, out_data_len + 1);
    memcpy(out_data_buf, out_data, out_data_len);

    // 释放内存
    rsa_util.Release();

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), out_data_buf, out_data_len);
    std::cout << hex_dump_buf << std::endl;

    unsigned char* origin_in_data = NULL;
    size_t origin_in_data_len = 0;

    if (rsa_util.PubDecode(&origin_in_data, origin_in_data_len, out_data_buf, out_data_len, padding) != 0)
    {
        rsa_util.Release();

        delete[] out_data_buf;
        out_data_buf = NULL;

        FAIL() << rsa_util.GetLastErrMsg();
    }

    std::cout << "out_data_len: " << out_data_len << ", origin_in_data_len: " << origin_in_data_len << std::endl;

    // 取出结果
    char* origin_in_data_buf = new char[origin_in_data_len + 1];
    memset(origin_in_data_buf, 0, origin_in_data_len + 1);
    memcpy(origin_in_data_buf, origin_in_data, origin_in_data_len);

    // 释放内存
    rsa_util.Release();

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), origin_in_data_buf, origin_in_data_len);
    std::cout << hex_dump_buf << std::endl;

    EXPECT_EQ(in_data_len, origin_in_data_len);
    EXPECT_STREQ((const char*) in_data, (const char*) origin_in_data_buf);

    delete[] origin_in_data_buf;
    origin_in_data_buf = NULL;

    delete[] out_data_buf;
    out_data_buf = NULL;
}

RSAUtilTest::RSAUtilTest()
{

}

RSAUtilTest::~RSAUtilTest()
{

}

void RSAUtilTest::SetUp()
{
    int ret = rsa_util_.Initialize("./rsa_public_key.pem", "./rsa_private_key.pem");
    if (ret != 0)
    {
        FAIL() << rsa_util_.GetLastErrMsg();
    }
}

void RSAUtilTest::TearDown()
{
    rsa_util_.Finalize();
}

/**
 * @brief 公钥加密私钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test001()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func1(rsa_util_, in_data, in_data_len, padding);
}

/**
 * @brief 公钥加密私钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size - 1
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test002()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE - 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func1(rsa_util_, in_data, in_data_len, padding);
}

/**
 * @brief 公钥加密私钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size + 1
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test003()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE + 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func1(rsa_util_, in_data, in_data_len, padding);
}

/**
 * @brief 公钥加密私钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size的整数倍
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test004()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE * 2;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func1(rsa_util_, in_data, in_data_len, padding);
}

/**
 * @brief 公钥加密私钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size的整数倍 - 1
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test005()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE * 2 - 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func1(rsa_util_, in_data, in_data_len, padding);
}

/**
 * @brief 公钥加密私钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size的整数倍 - 1
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test006()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE * 2 + 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func1(rsa_util_, in_data, in_data_len, padding);
}

/**
 * @brief 私钥加密公钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test007()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func2(rsa_util_, in_data, in_data_len, padding);
}

/**
 * @brief 私钥加密公钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size - 1
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test008()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE - 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func2(rsa_util_, in_data, in_data_len, padding);
}

/**
 * @brief 私钥加密公钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size + 1
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test009()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE + 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func2(rsa_util_, in_data, in_data_len, padding);
}

/**
 * @brief 私钥加密公钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size的整数倍
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test010()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE * 2;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func2(rsa_util_, in_data, in_data_len, padding);
}

/**
 * @brief 私钥加密公钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size的整数倍 - 1
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test011()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE * 2 - 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func2(rsa_util_, in_data, in_data_len, padding);
}

/**
 * @brief 私钥加密公钥解密
 * @details
 *  - Set Up:
 1,PADDING_PKCS1
 2,明文长度等于block_size的整数倍 + 1
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void RSAUtilTest::Test012()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE * 2 + 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func2(rsa_util_, in_data, in_data_len, padding);
}

void RSAUtilTest::Test013()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1_OAEP;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE + 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func1(rsa_util_, in_data, in_data_len, padding);
}

void RSAUtilTest::Test014()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_PKCS1_OAEP;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE + 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';

    unsigned char* out_data = NULL;
    size_t out_data_len = 0;

    EXPECT_TRUE(rsa_util_.PriEncode(&out_data, out_data_len, in_data, in_data_len, padding) != 0);
}

void RSAUtilTest::Test015()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_NO;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE + 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func1(rsa_util_, in_data, in_data_len, padding);
}

void RSAUtilTest::Test016()
{
    RSAUtil::Padding padding = RSAUtil::PADDING_NO;
    static const int BLOCK_SIZE = PRI_KEY_BITS / 8 - RSAUtil::GetPaddingSize(padding);

    const int DATA_LEN = BLOCK_SIZE + 1;
    unsigned char in_data[DATA_LEN + 1];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    in_data[DATA_LEN] = '\0';
    Func2(rsa_util_, in_data, in_data_len, padding);
}

ADD_TEST_F(RSAUtilTest, Test001);
ADD_TEST_F(RSAUtilTest, Test002);
ADD_TEST_F(RSAUtilTest, Test003);
ADD_TEST_F(RSAUtilTest, Test004);
ADD_TEST_F(RSAUtilTest, Test005);
ADD_TEST_F(RSAUtilTest, Test006);
ADD_TEST_F(RSAUtilTest, Test007);
ADD_TEST_F(RSAUtilTest, Test008);
ADD_TEST_F(RSAUtilTest, Test009);
ADD_TEST_F(RSAUtilTest, Test010);
ADD_TEST_F(RSAUtilTest, Test011);
ADD_TEST_F(RSAUtilTest, Test012);
ADD_TEST_F(RSAUtilTest, Test013);
ADD_TEST_F(RSAUtilTest, Test014);
ADD_TEST_F(RSAUtilTest, Test015);
ADD_TEST_F(RSAUtilTest, Test016);
