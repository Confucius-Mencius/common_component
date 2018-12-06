#include "byte_util.h"
#include "test_util.h"

namespace byte_util_test
{
/**
 * @brief bit设置与清除测试-u8类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test001()
{
    uint8_t v = 0;
    const int32_t BIT_ID_MIN = 1;
    const int32_t BIT_ID_MAX = 8;
    const int32_t BIT_ID_X = 4;

    // 最低位，set
    SET_BIT(v, BIT_ID_MIN);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，set
    SET_BIT(v, BIT_ID_MAX);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，set
    SET_BIT(v, BIT_ID_X);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_X));

    // 最低位，clear
    CLR_BIT(v, BIT_ID_MIN);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，clear
    CLR_BIT(v, BIT_ID_MAX);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，clear
    CLR_BIT(v, BIT_ID_X);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_X));
}

/**
 * @brief bit设置与清除测试-u16类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test002()
{
    uint16_t v = 0;
    const int32_t BIT_ID_MIN = 1;
    const int32_t BIT_ID_MAX = 16;
    const int32_t BIT_ID_X = 8;

    // 最低位，set
    SET_BIT(v, BIT_ID_MIN);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，set
    SET_BIT(v, BIT_ID_MAX);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，set
    SET_BIT(v, BIT_ID_X);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_X));

    // 最低位，clear
    CLR_BIT(v, BIT_ID_MIN);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，clear
    CLR_BIT(v, BIT_ID_MAX);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，clear
    CLR_BIT(v, BIT_ID_X);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_X));
}

/**
 * @brief bit设置与清除测试-u32类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test003()
{
    uint32_t v = 0;
    const int32_t BIT_ID_MIN = 1;
    const int32_t BIT_ID_MAX = 32;
    const int32_t BIT_ID_X = 16;

    // 最低位，set
    SET_BIT(v, BIT_ID_MIN);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，set
    SET_BIT(v, BIT_ID_MAX);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，set
    SET_BIT(v, BIT_ID_X);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_X));

    // 最低位，clear
    CLR_BIT(v, BIT_ID_MIN);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，clear
    CLR_BIT(v, BIT_ID_MAX);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，clear
    CLR_BIT(v, BIT_ID_X);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_X));
}

/**
 * @brief bit设置与清除测试-u64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test004()
{
    uint64_t v = 0;
    const int32_t BIT_ID_MIN = 1;
    const int32_t BIT_ID_MAX = 64;
    const int32_t BIT_ID_X = 32;

    // 最低位，set
    SET_BIT(v, BIT_ID_MIN);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，set
    SET_BIT(v, BIT_ID_MAX);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，set
    SET_BIT(v, BIT_ID_X);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_X));

    // 最低位，clear
    CLR_BIT(v, BIT_ID_MIN);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，clear
    CLR_BIT(v, BIT_ID_MAX);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，clear
    CLR_BIT(v, BIT_ID_X);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_X));
}

/**
 * @brief bit设置与清除测试-i8类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test005()
{
    int8_t v = 0;
    const int32_t BIT_ID_MIN = 1;
    const int32_t BIT_ID_MAX = 8;
    const int32_t BIT_ID_X = 4;

    // 最低位，set
    SET_BIT(v, BIT_ID_MIN);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，set
    SET_BIT(v, BIT_ID_MAX);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，set
    SET_BIT(v, BIT_ID_X);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_X));

    // 最低位，clear
    CLR_BIT(v, BIT_ID_MIN);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，clear
    CLR_BIT(v, BIT_ID_MAX);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，clear
    CLR_BIT(v, BIT_ID_X);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_X));
}

/**
 * @brief bit设置与清除测试-i16类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test006()
{
    int16_t v = 0;
    const int32_t BIT_ID_MIN = 1;
    const int32_t BIT_ID_MAX = 16;
    const int32_t BIT_ID_X = 8;

    // 最低位，set
    SET_BIT(v, BIT_ID_MIN);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，set
    SET_BIT(v, BIT_ID_MAX);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，set
    SET_BIT(v, BIT_ID_X);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_X));

    // 最低位，clear
    CLR_BIT(v, BIT_ID_MIN);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，clear
    CLR_BIT(v, BIT_ID_MAX);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，clear
    CLR_BIT(v, BIT_ID_X);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_X));
}

/**
 * @brief bit设置与清除测试-i32类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test007()
{
    int32_t v = 0;
    const int32_t BIT_ID_MIN = 1;
    const int32_t BIT_ID_MAX = 32;
    const int32_t BIT_ID_X = 16;

    // 最低位，set
    SET_BIT(v, BIT_ID_MIN);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，set
    SET_BIT(v, BIT_ID_MAX);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，set
    SET_BIT(v, BIT_ID_X);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_X));

    // 最低位，clear
    CLR_BIT(v, BIT_ID_MIN);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，clear
    CLR_BIT(v, BIT_ID_MAX);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，clear
    CLR_BIT(v, BIT_ID_X);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_X));
}

/**
 * @brief bit设置与清除测试-i64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test008()
{
    int64_t v = 0;
    const int32_t BIT_ID_MIN = 1;
    const int32_t BIT_ID_MAX = 64;
    const int32_t BIT_ID_X = 32;

    // 最低位，set
    SET_BIT(v, BIT_ID_MIN);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，set
    SET_BIT(v, BIT_ID_MAX);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，set
    SET_BIT(v, BIT_ID_X);
    EXPECT_TRUE(BIT_ENABLED(v, BIT_ID_X));

    // 最低位，clear
    CLR_BIT(v, BIT_ID_MIN);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MIN));

    // 最高位，clear
    CLR_BIT(v, BIT_ID_MAX);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_MAX));

    // 中间位，clear
    CLR_BIT(v, BIT_ID_X);
    EXPECT_FALSE(BIT_ENABLED(v, BIT_ID_X));
}

/**
 * @brief MAKEU16测试-u8类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test009()
{
    uint8_t i = 0xff;
    uint8_t j = 0x00;
    uint16_t ret = 0xff00;
    uint16_t n = MAKE_U16(i, j);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(i, U16_HIGH(n));
    EXPECT_EQ(j, U16_LOW(n));

    ret = 0x00ff;
    n = MAKE_U16(j, i);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(j, U16_HIGH(n));
    EXPECT_EQ(i, U16_LOW(n));
}

/**
 * @brief MAKEU32测试-u16类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test010()
{
    uint16_t i = 0xffff;
    uint16_t j = 0x0000;
    uint32_t ret = 0xffff0000;
    uint32_t n = MAKE_U32(i, j);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(i, U32_HIGH(n));
    EXPECT_EQ(j, U32_LOW(n));

    ret = 0x0000ffff;
    n = MAKE_U32(j, i);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(j, U32_HIGH(n));
    EXPECT_EQ(i, U32_LOW(n));
}

/**
 * @brief MAKEU64测试-u32类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test011()
{
    uint32_t i = 0xffffffff;
    uint32_t j = 0x00000000;
    uint64_t ret = UINT64_C(0xffffffff00000000);
    uint64_t n = MAKE_U64(i, j);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(i, U64_HIGH(n));
    EXPECT_EQ(j, U64_LOW(n));

    ret = UINT64_C(0x00000000ffffffff);
    n = MAKE_U64(j, i);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(j, U64_HIGH(n));
    EXPECT_EQ(i, U64_LOW(n));
}

/**
 * @brief MAKEU16测试-i8类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test012()
{
    int8_t i = 0x7f;
    int8_t j = 0x80;
    uint16_t ret = 0x7f80;
    uint16_t n = MAKE_U16(i, j);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(i, U16_HIGH(n));
    EXPECT_EQ(j, (int8_t) U16_LOW(n)); // 注意：EXPECT_EQ是比较int类型

    ret = 0x807f;
    n = MAKE_U16(j, i);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(j, (int8_t) U16_HIGH(n));
    EXPECT_EQ(i, U16_LOW(n));
}

/**
 * @brief MAKEU32测试-i16类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test013()
{
    int16_t i = 0x7fff;
    int16_t j = 0x8000;
    uint32_t ret = 0x7fff8000;
    uint32_t n = MAKE_U32(i, j);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(i, U32_HIGH(n));
    EXPECT_EQ(j, (int16_t) U32_LOW(n));

    ret = 0x80007fff;
    n = MAKE_U32(j, i);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(j, (int16_t) U32_HIGH(n));
    EXPECT_EQ(i, U32_LOW(n));
}

/**
 * @brief MAKEU64测试-i32类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test014()
{
    uint32_t i = 0x7fffffff;
    uint32_t j = 0x80000000;
    uint64_t ret = UINT64_C(0x7fffffff80000000);
    uint64_t n = MAKE_U64(i, j);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(i, U64_HIGH(n));
    EXPECT_EQ(j, U64_LOW(n));

    ret = UINT64_C(0x800000007fffffff);
    n = MAKE_U64(j, i);
    EXPECT_EQ(ret, n);
    EXPECT_EQ(j, U64_HIGH(n));
    EXPECT_EQ(i, U64_LOW(n));
}

ADD_TEST(ByteUtilTest, Test001);
ADD_TEST(ByteUtilTest, Test002);
ADD_TEST(ByteUtilTest, Test003);
ADD_TEST(ByteUtilTest, Test004);
ADD_TEST(ByteUtilTest, Test005);
ADD_TEST(ByteUtilTest, Test006);
ADD_TEST(ByteUtilTest, Test007);
ADD_TEST(ByteUtilTest, Test008);
ADD_TEST(ByteUtilTest, Test009);
ADD_TEST(ByteUtilTest, Test010);
ADD_TEST(ByteUtilTest, Test011);
ADD_TEST(ByteUtilTest, Test012);
ADD_TEST(ByteUtilTest, Test013);
ADD_TEST(ByteUtilTest, Test014);
}
