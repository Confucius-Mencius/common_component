#include "test_util.h"
#include "simple_str.h"

namespace simple_str_test
{
/**
 * @brief 字符串类测试-重载+、=运算符
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test001()
{
    SimpleStr<64> name;
    EXPECT_EQ(0, name.Len());

    const char s[] = "hgc";
    const char s1[] = "123";
    const char s2[] = "hgc123";
    const char s3[] = "hgc123hgc123";

    name = name + s;
    EXPECT_STREQ(s, name.Value());
    EXPECT_EQ(StrLen(s, sizeof(s)), name.Len());

    name = name + s1;

    EXPECT_STREQ(s2, name.Value());
    EXPECT_EQ(StrLen(s2, sizeof(s2)), name.Len());

    name = name + name;

    EXPECT_STREQ(s3, name.Value());
    EXPECT_EQ(StrLen(s3, sizeof(s3)), name.Len());
}

/**
 * @brief 字符串类测试-重载+=运算符
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test002()
{
    SimpleStr<64> name;

    const char s[] = "hgc";
    const char s1[] = "123";
    const char s2[] = "hgc123";
    const char s3[] = "hgc123hgc123";

    name += s;
    EXPECT_STREQ(s, name.Value());
    EXPECT_EQ(StrLen(s, sizeof(s)), name.Len());

    name += s1;

    EXPECT_STREQ(s2, name.Value());
    EXPECT_EQ(StrLen(s2, sizeof(s2)), name.Len());

    name += name;

    EXPECT_STREQ(s3, name.Value());
    EXPECT_EQ(StrLen(s3, sizeof(s3)), name.Len());
}

/**
 * @brief 溢出测试
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void Test003()
{
    SimpleStr<8> name;

    const char s[] = "hgc";
    const char s1[] = "123";
    const char s2[] = "hgc123";
    const char s3[] = "hgc123h";

    name += s;
    EXPECT_STREQ(s, name.Value());
    EXPECT_EQ(StrLen(s, sizeof(s)), name.Len());

    name += s1;

    EXPECT_STREQ(s2, name.Value());
    EXPECT_EQ(StrLen(s2, sizeof(s2)), name.Len());

    name += name;

    EXPECT_STREQ(s3, name.Value());
    EXPECT_EQ(StrLen(s3, sizeof(s3)), name.Len());
}

/**
 * @brief []测试
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void Test004()
{
    SimpleStr<8> s("0123");

    // 读
    EXPECT_EQ('0', s[0]);
    EXPECT_EQ('1', s[1]);
    EXPECT_EQ('2', s[2]);
    EXPECT_EQ('3', s[3]);
    EXPECT_EQ('\0', s[4]);
    EXPECT_EQ('\0', s[5]);

    // 写
    s[0] = 'a';
    EXPECT_EQ('a', s[0]);
}

/**
 * @brief 构造函数测试
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void Test005()
{
    SimpleStr<16> s1;
    EXPECT_EQ(0, s1.Len());
    EXPECT_STREQ("", s1.Value());

    SimpleStr<16> s2("xx");
    EXPECT_EQ(2, s2.Len());
    EXPECT_STREQ("xx", s2.Value());

    SimpleStr<16> s3('x', 2);
    EXPECT_EQ(2, s3.Len());
    EXPECT_STREQ("xx", s3.Value());

    SimpleStr<16> s4(s3);
    EXPECT_EQ(2, s4.Len());
    EXPECT_STREQ("xx", s4.Value());
}

ADD_TEST(SimpleStrTest, Test001);
ADD_TEST(SimpleStrTest, Test002);
ADD_TEST(SimpleStrTest, Test003);
ADD_TEST(SimpleStrTest, Test004);
ADD_TEST(SimpleStrTest, Test005);
} /* namespace simple_str_test */
