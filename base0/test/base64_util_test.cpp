#include "base64_util_test.h"
#include "base64_util.h"

static void Func(int with_newline)
{
    unsigned char in_data[128] = "";
    size_t in_data_len = sizeof(in_data) - 1;

    unsigned char* out_data = NULL;
    size_t out_data_len = 0;

    int ret = Base64Encode(&out_data, out_data_len, in_data, in_data_len, with_newline); // with newline编码
    ASSERT_EQ(0, ret);

    std::cout << out_data_len << std::endl;
    std::cout << out_data << std::endl;

    unsigned char* in_data2 = new unsigned char[out_data_len + 1];
    in_data2[out_data_len] = '\0';
    memcpy(in_data2, out_data, out_data_len);
    size_t in_data_len2 = out_data_len;

    Base64Release(&out_data);
    out_data = NULL;

    unsigned char* out_data2 = NULL;
    size_t out_data_len2 = 0;

    ret = Base64Decode(&out_data2, out_data_len2, in_data2, in_data_len2, with_newline); // with newline解码
    ASSERT_EQ(0, ret);

    EXPECT_EQ(in_data_len, out_data_len2);
    EXPECT_STREQ((const char*) in_data, (const char*) out_data2);

    Base64Release(&out_data2);
    out_data2 = NULL;

    unsigned char* out_data3 = NULL;
    size_t out_data_len3 = 0;

    ret = Base64Decode(&out_data3, out_data_len3, in_data2, in_data_len2, !with_newline); // 不用with newline解码
    ASSERT_EQ(0, ret);

    EXPECT_NE(in_data_len, out_data_len3);
    EXPECT_STRNE((const char*) in_data, (const char*) out_data3);

    Base64Release(&out_data3);
    out_data3 = NULL;
}

Base64UtilTest::Base64UtilTest()
{

}

Base64UtilTest::~Base64UtilTest()
{

}

void Base64UtilTest::Test001()
{
    Func(1);
}

void Base64UtilTest::Test002()
{
    Func(0);
}

ADD_TEST_F(Base64UtilTest, Test001);
ADD_TEST_F(Base64UtilTest, Test002);
