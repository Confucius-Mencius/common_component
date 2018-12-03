#include "test_util.h"
#include "num_str_util.h"

namespace num_str_util_test
{
void NumToStrTest()
{
    // i8
    char buf1[8] = "";
    int n = Num2Str(buf1, sizeof(buf1), Type2Type<i32>(), (i8) 127);
    ASSERT_EQ(3, n);
    EXPECT_STREQ("127", buf1);

    // u8
    char buf2[8] = "";
    n = Num2Str(buf2, sizeof(buf2), Type2Type<u32>(), (u8) 255);
    ASSERT_EQ(3, n);
    EXPECT_STREQ("255", buf2);

    // i16
    char buf3[8] = "";
    n = Num2Str(buf3, sizeof(buf3), Type2Type<i32>(), (i16) 32767);
    ASSERT_EQ(5, n);
    EXPECT_STREQ("32767", buf3);

    // u16
    char buf4[8] = "";
    n = Num2Str(buf4, sizeof(buf4), Type2Type<u32>(), (u16) 65535);
    ASSERT_EQ(5, n);
    EXPECT_STREQ("65535", buf4);

    // i32
    char buf5[16] = "";
    n = Num2Str(buf5, sizeof(buf5), Type2Type<i32>(), 2147483647);
    ASSERT_EQ(10, n);
    EXPECT_STREQ("2147483647", buf5);

    // u32
    char buf6[16] = "";
    n = Num2Str(buf6, sizeof(buf6), Type2Type<u32>(), UINT32_C(4294967295));
    ASSERT_EQ(10, n);
    EXPECT_STREQ("4294967295", buf6);

    // i64
    char buf7[24] = "";
    n = Num2Str(buf7, sizeof(buf7), Type2Type<i64>(), INT64_C(9223372036854775807));
    ASSERT_EQ(19, n);
    EXPECT_STREQ("9223372036854775807", buf7);

    // u64
    char buf8[24] = "";
    n = Num2Str(buf8, sizeof(buf8), Type2Type<u64>(), UINT64_C(18446744073709551615));
    ASSERT_EQ(20, n);
    EXPECT_STREQ("18446744073709551615", buf8);

    // f32
    char buf9_1_1[8] = "";
    n = Num2Str(buf9_1_1, sizeof(buf9_1_1), Type2Type<f64>(), 12.0f);
    ASSERT_EQ(7, n);
    EXPECT_STREQ("12.0000", buf9_1_1);

    char buf9_1_2[5] = "";
    n = Num2Str(buf9_1_2, sizeof(buf9_1_2), Type2Type<f64>(), 12.0f);
    ASSERT_EQ(4, n);
    EXPECT_STREQ("12.0", buf9_1_2);

    char buf9_1_3[11] = "";
    n = Num2Str(buf9_1_3, sizeof(buf9_1_3), Type2Type<f64>(), 12.0f);
    ASSERT_EQ(9, n);
    EXPECT_STREQ("12.000000", buf9_1_3); // 最多保留6位小数

    char buf9_2_1[8] = "";
    n = Num2Str(buf9_2_1, sizeof(buf9_2_1), Type2Type<f64>(), 12.01234567f);
    ASSERT_EQ(7, n);
    EXPECT_STREQ("12.0123", buf9_2_1);

    char buf9_2_2[11] = "";
    n = Num2Str(buf9_2_2, sizeof(buf9_2_2), Type2Type<f64>(), 12.01234567f);
    ASSERT_EQ(9, n);
    EXPECT_STREQ("12.012345", buf9_2_2);

    char buf9_3[8] = "";
    n = Num2Str(buf9_3, sizeof(buf9_3), Type2Type<f64>(), 123.00000001f);
    ASSERT_EQ(7, n);
    EXPECT_STREQ("123.000", buf9_3);

    // f64
    char buf10_1[8] = "";
    n = Num2Str(buf10_1, sizeof(buf10_1), Type2Type<f64>(), 12.0);
    ASSERT_EQ(7, n);
    EXPECT_STREQ("12.0000", buf10_1);

    char buf10_2_1[8] = "";
    n = Num2Str(buf10_2_1, sizeof(buf10_2_1), Type2Type<f64>(), 12.01234567);
    ASSERT_EQ(7, n);
    EXPECT_STREQ("12.0123", buf10_2_1);

    char buf10_2_2[11] = "";
    n = Num2Str(buf10_2_2, sizeof(buf10_2_2), Type2Type<f64>(), 12.01234567);
    ASSERT_EQ(9, n);
    EXPECT_STREQ("12.012346", buf10_2_2);

    char buf10_3[12] = "";
    n = Num2Str(buf10_3, sizeof(buf10_3), Type2Type<f64>(), 123.00000001);
    ASSERT_EQ(10, n);
    EXPECT_STREQ("123.000000", buf10_3);
}

void StrToNumTest()
{
    // i32
    const char str1_1[] = "1234";
    i32 i1_1 = Str2Num(Type2Type<i32>(), str1_1);
    EXPECT_EQ(1234, i1_1);

    const char str1_2[] = "00101234";
    i32 i1_2 = Str2Num(Type2Type<i32>(), str1_2);
    EXPECT_EQ(101234, i1_2); // 去掉开头的0

    const char str1_3[] = "x10y01z1234";
    i32 i1_3 = Str2Num(Type2Type<i32>(), str1_3);
    EXPECT_EQ(0, i1_3); // 以非数字开头时，结果为0

    const char str1_4[] = "00x1234";
    i32 i1_4 = Str2Num(Type2Type<i32>(), str1_4);
    EXPECT_EQ(0, i1_4);

    // u32
    const char str2[] = "1234";
    u32 u1 = Str2Num(Type2Type<u32>(), str2);
    EXPECT_EQ((u32) 1234, u1);

    // i64
    const char str3[] = "1234567890123456";
    i64 i2 = Str2Num(Type2Type<i64>(), str3);
    EXPECT_EQ(INT64_C(1234567890123456), i2);

    // u64
    const char str4[] = "1234567890123456";
    u64 u2 = Str2Num(Type2Type<u64>(), str4);
    EXPECT_EQ(UINT64_C(1234567890123456), u2);

    // f32
    const char str5[] = "1234.56";
    f32 f1 = Str2Num(Type2Type<f64>(), str5);
    EXPECT_FLOAT_EQ(1234.56f, f1);

    // f64
    const char str6[] = "1234567890123456.78";
    f64 f2 = Str2Num(Type2Type<f64>(), str6);
    EXPECT_DOUBLE_EQ(1234567890123456.78, f2);
}

ADD_TEST(NumStrUtilTest, NumToStrTest);
ADD_TEST(NumStrUtilTest, StrToNumTest);
} // namespace num_str_util_test
