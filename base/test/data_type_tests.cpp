#include "data_types.h"
#include "simple_log.h"
#include "test_util.h"

namespace data_types_test
{
/**
 * @brief 各种基础数据类型的最大、小值测试。
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
    // i8
    //EXPECT_EQ(INT8_C(0x80), I8_MIN);
    EXPECT_EQ(std::numeric_limits<i8>::min(), I8_MIN);
    EXPECT_EQ(std::numeric_limits<i8>::max(), I8_MAX);

    // u8
    EXPECT_EQ(std::numeric_limits<u8>::min(), U8_MIN);
    EXPECT_EQ(std::numeric_limits<u8>::max(), U8_MAX);

    // i16
    //EXPECT_EQ(INT16_C(0x8000), I16_MIN);
    EXPECT_EQ(std::numeric_limits<i16>::min(), I16_MIN);
    EXPECT_EQ(std::numeric_limits<i16>::max(), I16_MAX);

    // u16
    EXPECT_EQ(std::numeric_limits<u16>::min(), U16_MIN);
    EXPECT_EQ(std::numeric_limits<u16>::max(), U16_MAX);

    // i32
    EXPECT_EQ(std::numeric_limits<i32>::min(), I32_MIN);
    EXPECT_EQ(std::numeric_limits<i32>::max(), I32_MAX);

    // u32
    EXPECT_EQ(std::numeric_limits<u32>::min(), U32_MIN);
    EXPECT_EQ(std::numeric_limits<u32>::max(), U32_MAX);

    // i64
    EXPECT_EQ(std::numeric_limits<i64>::min(), I64_MIN);
    EXPECT_EQ(std::numeric_limits<i64>::max(), I64_MAX);

    // u64
    EXPECT_EQ(std::numeric_limits<u64>::min(), U64_MIN);
    EXPECT_EQ(std::numeric_limits<u64>::max(), U64_MAX);

    // f32
    EXPECT_EQ(-std::numeric_limits<f32>::max(), F32_MIN);
    EXPECT_EQ(std::numeric_limits<f32>::max(), F32_MAX);

    // f64
    EXPECT_EQ(-std::numeric_limits<f64>::max(), F64_MIN);
    EXPECT_EQ(std::numeric_limits<f64>::max(), F64_MAX);

    // long
    EXPECT_EQ(std::numeric_limits<long>::min(), LONG_MIN);
    EXPECT_EQ(std::numeric_limits<long>::max(), LONG_MAX);

    // ulong
    EXPECT_EQ(std::numeric_limits<ulong>::min(), ULONG_MIN);
    EXPECT_EQ(std::numeric_limits<ulong>::max(), ULONG_MAX);
}

/*
 INT_MIN in <limits.h> is a macro that expands to the minimum value for an object of type int. In the 32-bit C compilers I have installed at the moment, it is defined as:

 #define INT_MIN     (-2147483647 - 1)
 So what exactly is wrong with the integer constant -2147483648 ?

 Well, firstly it is not an integer constant. Let’s see what the standard says:

 “An integer constant begins with a digit, but has no period or exponent part. It may have a prefix that specifies its base and a suffix that specifies its type.”
 You will notice there is no mention of a sign. So -2147483648 is in fact a constant expression, consisting of the unary minus operator, and the integer constant 2147483648.

 This still does not explain why that expression is not used directly in the macro. To see that, we have to revisit the rules for the type of integer constants.

 The type of an unsuffixed integer constant is the first of these in which its value can be represented:

 C89 :   int, long int, unsigned long int
 C99 :   int, long int, long long int
 C++ :   int, long int, long long int
 The problem is that 2147483648 cannot be represented in a signed 32-bit integer, so it becomes either an unsigned long int or a long long int.

 So we have to resort to a little trickery, and compute -2147483648 as (-2147483647 – 1), which all fit nicely into 32-bit signed integers, and INT_MIN gets the right type and value.

 If you happen to look up INT_MIN in the standard you will see:

 minimum value for an object of type int

 INT_MIN                 -32767
 Which brings up the question why isn’t it (-32767 – 1)?

 Pretty much any computer available today uses two’s complement to represent signed numbers, but this hasn’t always been the case.

 Since C was designed to work efficiently on a variety of architectures, the standard’s limits allow for using other representations as well.

 I will end this post with a little (not quite standard conformant) example. Try compiling it with your favorite C compiler, and let us know if something puzzles you.
 */

//int main(void)
//{
//    // 在vc++2008下，下列if都是true
//    if (-2147483648 > 0)     printf("positive\n");
//    if (-2147483647 - 1 < 0) printf("negative\n");
//    if (INT_MIN == -INT_MIN) printf("equal\n");
//    if (FLT_MIN > 0)         printf("floating\n");
//
//    return 0;
//}
void IntMinTest()
{
    EXPECT_FALSE(-2147483648 > 0);
    EXPECT_TRUE(-2147483647 - 1 < 0);
//    EXPECT_TRUE(INT_MIN == -INT_MIN); // gcc7.3.0: overflow
    EXPECT_TRUE(FLT_MIN > 0);
}

ADD_TEST(DataTypesTest, Test001);
ADD_TEST(DataTypesTest, IntMinTest);
} /* namespace data_type_test */
