#include "conv_test.h"
#include "utf8_trad_simp_conv.h"
#include "utf8_unicode_conv.h"

ConvTest::ConvTest()
{
}

ConvTest::~ConvTest()
{
}

void ConvTest::UTF8UnicodeConvTest()
{
    char src[] = "我爱你";
    size_t src_bytes = strlen(src); // strlen(src)==9, sizeof(src)==10

    std::cout << src_bytes << " " << sizeof(src) << std::endl;

    size_t dst_buf_size = 16;
    wchar_t dst[dst_buf_size]; // 一个wchar_t占4个字节
    memset(dst, 0, sizeof(dst));

    UTF8UnicodeConv utf8_unicode_conv_util;
    if (utf8_unicode_conv_util.UTF82Unicode(dst, dst_buf_size, src, src_bytes) != 0)
    {
        std::cout << utf8_unicode_conv_util.GetLastErrMsg() << std::endl;
        FAIL();
    }

    std::wcout << dst << " " << wcslen(dst) << " " << sizeof(dst) << std::endl; // wcslen(dst)==3, sizeof(dst)==64

    size_t src1_buf_size = 16;
    char src1[src1_buf_size];
    memset(src1, 0, sizeof(src1));

    if (utf8_unicode_conv_util.Unicode2UTF8(src1, src1_buf_size, dst, wcslen(dst) * sizeof(wchar_t)) != 0)
    {
        std::cout << utf8_unicode_conv_util.GetLastErrMsg() << std::endl;
        FAIL();
    }

    std::cout << src1 << " " << strlen(src1) << " " << sizeof(src1) << std::endl;
    EXPECT_STREQ(src, src1);
}

void ConvTest::UTF8TradSimpConvTest()
{
    char src[] = "中华民国";
    size_t src_len = strlen(src); // strlen(src)==12, sizeof(src)==13

    std::cout << src_len << " " << sizeof(src) << std::endl;

    size_t dst_len = 16;
    char dst[dst_len];
    memset(dst, 0, sizeof(dst));

    UTF8TradSimpConv utf8_trad_simp_conv_util;
    if (utf8_trad_simp_conv_util.Simp2Trad(dst, dst_len, src, src_len) != 0)
    {
        std::cout << utf8_trad_simp_conv_util.GetLastErrMsg() << std::endl;
        FAIL();
    }

    std::cout << dst << " " << dst_len << " " << strlen(dst) << " " << sizeof(dst) <<
              std::endl; // dst_len==12, strlen(dst)==12, sizeof(dst)==16

    size_t src1_len = 16;
    char src1[src1_len];
    memset(src1, 0, sizeof(src1));

    if (utf8_trad_simp_conv_util.Trad2Simp(src1, src1_len, dst, dst_len) != 0)
    {
        std::cout << utf8_trad_simp_conv_util.GetLastErrMsg() << std::endl;
        FAIL();
    }

    std::cout << src1 << " " << src1_len << " " << strlen(src1) << " " << sizeof(src1) << std::endl;
    EXPECT_STREQ(src, src1);
}

void ConvTest::UTFUnicodeConvTest()
{
    UTF8UnicodeConv conv;

    wchar_t dst_buf[1024];
    std::cout << sizeof(dst_buf) << std::endl;

    char src_buf[] = "好好学习";
    int ret = conv.UTF82Unicode(dst_buf, sizeof(dst_buf), src_buf, sizeof(src_buf) - 1);
    ASSERT_EQ(0, ret);
    std::wcout << dst_buf << ", " << wcslen(dst_buf) << std::endl;
}

ADD_TEST_F(ConvTest, UTF8UnicodeConvTest);
ADD_TEST_F(ConvTest, UTF8TradSimpConvTest);
ADD_TEST_F(ConvTest, UTFUnicodeConvTest);
