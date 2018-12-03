#include "aes_util_test.h"
#include "aes_util.h"
#include "hex_dump.h"

static void Func(const unsigned char* in_data, size_t in_data_len, const unsigned char* passwd, AESPasswdLen passwd_len)
{
    char hex_dump_buf[4096] = "";

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), in_data, in_data_len);
    std::cout << hex_dump_buf << std::endl;

    unsigned char* out_data = NULL;
    size_t out_data_len = 0;

    // out_data_len >= in_data_len
    if (AESEncode(&out_data, out_data_len, in_data, in_data_len, passwd, passwd_len) != 0)
    {
        AESRelease(&out_data);
        FAIL();
    }

    //  取出结果
    unsigned char* out_data_buf = new unsigned char[out_data_len + 1];
    out_data_buf[out_data_len] = '\0';
    memcpy(out_data_buf, out_data, out_data_len);

    // 释放内存
    AESRelease(&out_data);

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), out_data_buf, out_data_len);
    std::cout << hex_dump_buf << std::endl;

    // 注意这里解密分配的空间大小应该为加密之后的数据长度,因为加密时会填充,加密后的数据长度比原始数据长度大
    unsigned char* origin_in_data = new unsigned char[out_data_len + 1];
    origin_in_data[out_data_len] = '\0';
    memset(origin_in_data, 0, in_data_len);

    if (AESDecode(origin_in_data, out_data_buf, out_data_len, passwd, passwd_len) != 0)
    {
        delete[] origin_in_data;
        origin_in_data = NULL;
        FAIL();
    }

    std::cout << "out_data_len: " << out_data_len << ", in_data_len: " << in_data_len << std::endl;

    std::string s((const char*) origin_in_data, in_data_len); // 结果应该截断为in_data_len，即加密之前的数据长度
    std::cout << "s: " << s << ", s.size(): " << s.size() << std::endl;

    EXPECT_STREQ((const char*) in_data, s.c_str());

    memset(hex_dump_buf, 0, sizeof(hex_dump_buf));
    HexDump(hex_dump_buf, sizeof(hex_dump_buf), origin_in_data, in_data_len);
    std::cout << hex_dump_buf << std::endl;

    delete[] origin_in_data;
    origin_in_data = NULL;
}

AESUtilTest::AESUtilTest()
{

}

AESUtilTest::~AESUtilTest()
{

}

/**
 * @brief
 * @details
 *  - Set Up:
 1,秘钥为16字节
 2,明文长度为AES_BLOCK_SIZE的整数倍
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void AESUtilTest::Test001()
{
    // 16字节的秘钥
    unsigned char passwd[AES_PASSWD_BIT_128 / 8] = "";
    for (int i = 0; i < (int) sizeof(passwd); ++i)
    {
        passwd[i] = 'A' + rand() % 26;
    }

    unsigned char in_data[AES_BLOCK_SIZE * 2 + 1] = "";
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    Func(in_data, in_data_len, passwd, AES_PASSWD_BIT_128);
}

/**
 * @brief
 * @details
 *  - Set Up:
 1,秘钥为16字节
 2,明文长度为AES_BLOCK_SIZE的整数倍 - 1
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void AESUtilTest::Test002()
{
    // 16字节的秘钥
    unsigned char passwd[AES_PASSWD_BIT_128 / 8] = "";
    for (int i = 0; i < (int) sizeof(passwd); ++i)
    {
        passwd[i] = 'A' + rand() % 26;
    }

    unsigned char in_data[AES_BLOCK_SIZE * 2];
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    Func(in_data, in_data_len, passwd, AES_PASSWD_BIT_128);
}

/**
 * @brief
 * @details
 *  - Set Up:
 1,秘钥为16字节
 2,明文长度为AES_BLOCK_SIZE的整数倍 + 1
 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void AESUtilTest::Test003()
{
    // 16字节的秘钥
    unsigned char passwd[AES_PASSWD_BIT_128 / 8] = "";
    for (size_t i = 0; i < (int) sizeof(passwd); ++i)
    {
        passwd[i] = 'A' + rand() % 26;
    }

    unsigned char in_data[AES_BLOCK_SIZE * 2 + 1 + 1] = "";
    size_t in_data_len = sizeof(in_data) - 1;

    for (size_t i = 0; i < in_data_len; ++i)
    {
        in_data[i] = rand() % 256;
    }

    Func(in_data, in_data_len, passwd, AES_PASSWD_BIT_128);
}

/**
 * @brief 一个真实的秘钥和数据
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void AESUtilTest::Test004()
{
    unsigned char passwd[] = "a@flp5q^rf6iqm9j";

    unsigned char in_data[] = "1422587769-0-anystore";
    size_t in_data_len = sizeof(in_data) - 1;

    Func(in_data, in_data_len, passwd, AES_PASSWD_BIT_128);
}

ADD_TEST_F(AESUtilTest, Test001);
ADD_TEST_F(AESUtilTest, Test002);
ADD_TEST_F(AESUtilTest, Test003);
ADD_TEST_F(AESUtilTest, Test004);
