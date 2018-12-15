#include "num_util.h"
#include "test_util.h"

namespace num_util_test
{
/**
 * @brief 取整计算测试-i32类型
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
    // 被除数为正数，除数为正数
    // 能整除
    i32 a = 100;
    i32 b = 5;
    i32 ceil_val = 20;
    i32 floor_val = 20;
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 不能整除
    a = 100;
    b = 6;
    ceil_val = 17;
    floor_val = 16;
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 被除数为正数，除数为负数
    // 能整除
    a = 100;
    b = -5;
    ceil_val = -20;
    floor_val = -20;
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 不能整除
    a = 100;
    b = -6;
    ceil_val = -16;
    floor_val = -17;
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 被除数为负数，除数为正数
    // 能整除
    a = -100;
    b = 5;
    ceil_val = -20;
    floor_val = -20;
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 不能整除
    a = -100;
    b = 6;
    ceil_val = -16;
    floor_val = -17;
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 被除数为负数，除数为负数
    // 能整除
    a = -100;
    b = -5;
    ceil_val = 20;
    floor_val = 20;
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 不能整除
    a = -100;
    b = -6;
    ceil_val = 17;
    floor_val = 16;
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));
}

/**
 * @brief 取整计算测试-u32类型
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
    // 能整除
    u32 a = UINT32_C(100);
    u32 b = UINT32_C(5);
    u32 ceil_val = UINT32_C(20);
    u32 floor_val = UINT32_C(20);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 不能整除
    a = UINT32_C(100);
    b = UINT32_C(6);
    ceil_val = UINT32_C(17);
    floor_val = UINT32_C(16);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));
}

/**
 * @brief 取整计算测试-i64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test003()
{
    // 被除数为正数，除数为正数
    // 能整除
    i64 a = INT64_C(100);
    i64 b = INT64_C(5);
    i64 ceil_val = INT64_C(20);
    i64 floor_val = INT64_C(20);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 不能整除
    a = INT64_C(100);
    b = INT64_C(6);
    ceil_val = INT64_C(17);
    floor_val = INT64_C(16);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 被除数为正数，除数为负数
    // 能整除
    a = INT64_C(100);
    b = INT64_C(-5);
    ceil_val = INT64_C(-20);
    floor_val = INT64_C(-20);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 不能整除
    a = INT64_C(100);
    b = INT64_C(-6);
    ceil_val = INT64_C(-16);
    floor_val = INT64_C(-17);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 被除数为负数，除数为正数
    // 能整除
    a = INT64_C(-100);
    b = INT64_C(5);
    ceil_val = INT64_C(-20);
    floor_val = INT64_C(-20);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 不能整除
    a = INT64_C(-100);
    b = INT64_C(6);
    ceil_val = INT64_C(-16);
    floor_val = INT64_C(-17);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 被除数为负数，除数为负数
    // 能整除
    a = INT64_C(-100);
    b = INT64_C(-5);
    ceil_val = INT64_C(20);
    floor_val = INT64_C(20);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 不能整除
    a = INT64_C(-100);
    b = INT64_C(-6);
    ceil_val = INT64_C(17);
    floor_val = INT64_C(16);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));
}

/**
 * @brief 取整计算测试-u64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test004()
{
    // 能整除
    u64 a = UINT64_C(100);
    u64 b = UINT64_C(5);
    u64 ceil_val = UINT64_C(20);
    u64 floor_val = UINT64_C(20);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));

    // 不能整除
    a = UINT64_C(100);
    b = UINT64_C(6);
    ceil_val = UINT64_C(17);
    floor_val = UINT64_C(16);
    EXPECT_EQ(ceil_val, Ceil(a, b));
    EXPECT_EQ(floor_val, Floor(a, b));
}

/**
 * @brief swap测试-i32类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test005()
{
    // a为正数，b为正数
    i32 a = 100;
    i32 b = 200;
    i32 old_a = a;
    i32 old_b = b;

    Swap(a, b);
    EXPECT_EQ(old_b, a);
    EXPECT_EQ(old_a, b);

    // a为正数，b为负数
    a = 100;
    b = -200;
    old_a = a;
    old_b = b;

    Swap(a, b);
    EXPECT_EQ(old_b, a);
    EXPECT_EQ(old_a, b);

    // a为负数，b为负数
    a = -100;
    b = -200;
    old_a = a;
    old_b = b;

    Swap(a, b);
    EXPECT_EQ(old_b, a);
    EXPECT_EQ(old_a, b);
}

/**
 * @brief swap测试-u32类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test006()
{
    u32 a = UINT32_C(100);
    u32 b = UINT32_C(200);
    u32 old_a = a;
    u32 old_b = b;

    Swap(a, b);
    EXPECT_EQ(old_b, a);
    EXPECT_EQ(old_a, b);
}

/**
 * @brief swap测试-i64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test007()
{
    // a为正数，b为正数
    i64 a = INT64_C(100);
    i64 b = INT64_C(200);
    i64 old_a = a;
    i64 old_b = b;

    Swap(a, b);
    EXPECT_EQ(old_b, a);
    EXPECT_EQ(old_a, b);

    // a为正数，b为负数
    a = INT64_C(100);
    b = INT64_C(-200);
    old_a = a;
    old_b = b;

    Swap(a, b);
    EXPECT_EQ(old_b, a);
    EXPECT_EQ(old_a, b);

    // a为负数，b为负数
    a = INT64_C(-100);
    b = INT64_C(-200);
    old_a = a;
    old_b = b;

    Swap(a, b);
    EXPECT_EQ(old_b, a);
    EXPECT_EQ(old_a, b);
}

/**
 * @brief swap测试-u64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test008()
{
    u64 a = UINT64_C(100);
    u64 b = UINT64_C(200);
    u64 old_a = a;
    u64 old_b = b;

    Swap(a, b);
    EXPECT_EQ(old_b, a);
    EXPECT_EQ(old_a, b);
}

/**
 * @brief 四舍五入测试-f32类型。
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test009()
{
    f32 f;
    f32 ret;

    // 1.11f
    f = 1.11f;

    ret = 1;
    EXPECT_FLOAT_EQ(ret, Round(f, 0));

    ret = 1.1f;
    EXPECT_FLOAT_EQ(ret, Round(f, 1));

    ret = 1.11f;
    EXPECT_FLOAT_EQ(ret, Round(f, 2));

    ret = 1.110f;
    EXPECT_FLOAT_EQ(ret, Round(f, 3));

    // 1.15f
    f = 1.15f;

    ret = 1;
    EXPECT_FLOAT_EQ(ret, Round(f, 0));

    ret = 1.2f;
    EXPECT_FLOAT_EQ(ret, Round(f, 1));

    ret = 1.15f;
    EXPECT_FLOAT_EQ(ret, Round(f, 2));

    ret = 1.150f;
    EXPECT_FLOAT_EQ(ret, Round(f, 3));

    // 1.16f
    f = 1.16f;

    ret = 1;
    EXPECT_FLOAT_EQ(ret, Round(f, 0));

    ret = 1.2f;
    EXPECT_FLOAT_EQ(ret, Round(f, 1));

    ret = 1.16f;
    EXPECT_FLOAT_EQ(ret, Round(f, 2));

    ret = 1.160f;
    EXPECT_FLOAT_EQ(ret, Round(f, 3));

    // 1.51f
    f = 1.51f;

    ret = 2;
    EXPECT_FLOAT_EQ(ret, Round(f, 0));

    ret = 1.5f;
    EXPECT_FLOAT_EQ(ret, Round(f, 1));

    ret = 1.51f;
    EXPECT_FLOAT_EQ(ret, Round(f, 2));

    ret = 1.510f;
    EXPECT_FLOAT_EQ(ret, Round(f, 3));

    // 1.55f
    f = 1.55f;

    ret = 2;
    EXPECT_FLOAT_EQ(ret, Round(f, 0));

    ret = 1.6f;
    EXPECT_FLOAT_EQ(ret, Round(f, 1));

    ret = 1.55f;
    EXPECT_FLOAT_EQ(ret, Round(f, 2));

    ret = 1.550f;
    EXPECT_FLOAT_EQ(ret, Round(f, 3));

    // 负数
    // -1.11f
    f = -1.11f;

    ret = -1;
    EXPECT_FLOAT_EQ(ret, Round(f, 0));

    ret = -1.1f;
    EXPECT_FLOAT_EQ(ret, Round(f, 1));

    ret = -1.11f;
    EXPECT_FLOAT_EQ(ret, Round(f, 2));

    ret = -1.110f;
    EXPECT_FLOAT_EQ(ret, Round(f, 3));

    // -1.15f
    f = -1.15f;

    ret = -1;
    EXPECT_FLOAT_EQ(ret, Round(f, 0));

    ret = -1.2f;
    EXPECT_FLOAT_EQ(ret, Round(f, 1));

    ret = -1.15f;
    EXPECT_FLOAT_EQ(ret, Round(f, 2));

    ret = -1.150f;
    EXPECT_FLOAT_EQ(ret, Round(f, 3));

    // -1.16f
    f = -1.16f;

    ret = -1;
    EXPECT_FLOAT_EQ(ret, Round(f, 0));

    ret = -1.2f;
    EXPECT_FLOAT_EQ(ret, Round(f, 1));

    ret = -1.16f;
    EXPECT_FLOAT_EQ(ret, Round(f, 2));

    ret = -1.160f;
    EXPECT_FLOAT_EQ(ret, Round(f, 3));

    // -1.51f
    f = -1.51f;

    ret = -2;
    EXPECT_FLOAT_EQ(ret, Round(f, 0));

    ret = -1.5f;
    EXPECT_FLOAT_EQ(ret, Round(f, 1));

    ret = -1.51f;
    EXPECT_FLOAT_EQ(ret, Round(f, 2));

    ret = -1.510f;
    EXPECT_FLOAT_EQ(ret, Round(f, 3));

    // -1.55f
    f = -1.55f;

    ret = -2;
    EXPECT_FLOAT_EQ(ret, Round(f, 0));

    ret = -1.6f;
    EXPECT_FLOAT_EQ(ret, Round(f, 1));

    ret = -1.55f;
    EXPECT_FLOAT_EQ(ret, Round(f, 2));

    ret = -1.550f;
    EXPECT_FLOAT_EQ(ret, Round(f, 3));
}

/**
 * @brief 四舍五入测试-f64类型。
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test010()
{
    f64 f;
    f64 ret;

    // 1.11
    f = 1.11;

    ret = 1;
    EXPECT_DOUBLE_EQ(ret, Round(f, 0));

    ret = 1.1;
    EXPECT_DOUBLE_EQ(ret, Round(f, 1));

    ret = 1.11;
    EXPECT_DOUBLE_EQ(ret, Round(f, 2));

    ret = 1.110;
    EXPECT_DOUBLE_EQ(ret, Round(f, 3));

    // 1.15
    f = 1.15;

    ret = 1;
    EXPECT_DOUBLE_EQ(ret, Round(f, 0));

    ret = 1.2;
    EXPECT_DOUBLE_EQ(ret, Round(f, 1));

    ret = 1.15;
    EXPECT_DOUBLE_EQ(ret, Round(f, 2));

    ret = 1.150;
    EXPECT_DOUBLE_EQ(ret, Round(f, 3));

    // 1.16
    f = 1.16;

    ret = 1;
    EXPECT_DOUBLE_EQ(ret, Round(f, 0));

    ret = 1.2;
    EXPECT_DOUBLE_EQ(ret, Round(f, 1));

    ret = 1.16;
    EXPECT_DOUBLE_EQ(ret, Round(f, 2));

    ret = 1.160;
    EXPECT_DOUBLE_EQ(ret, Round(f, 3));

    // 1.51
    f = 1.51;

    ret = 2;
    EXPECT_DOUBLE_EQ(ret, Round(f, 0));

    ret = 1.5;
    EXPECT_DOUBLE_EQ(ret, Round(f, 1));

    ret = 1.51;
    EXPECT_DOUBLE_EQ(ret, Round(f, 2));

    ret = 1.510;
    EXPECT_DOUBLE_EQ(ret, Round(f, 3));

    // 1.55
    f = 1.55;

    ret = 2;
    EXPECT_DOUBLE_EQ(ret, Round(f, 0));

    ret = 1.6;
    EXPECT_DOUBLE_EQ(ret, Round(f, 1));

    ret = 1.55;
    EXPECT_DOUBLE_EQ(ret, Round(f, 2));

    ret = 1.550;
    EXPECT_DOUBLE_EQ(ret, Round(f, 3));

    // 负数
    // -1.11
    f = -1.11;

    ret = -1;
    EXPECT_DOUBLE_EQ(ret, Round(f, 0));

    ret = -1.1;
    EXPECT_DOUBLE_EQ(ret, Round(f, 1));

    ret = -1.11;
    EXPECT_DOUBLE_EQ(ret, Round(f, 2));

    ret = -1.110;
    EXPECT_DOUBLE_EQ(ret, Round(f, 3));

    // -1.15
    f = -1.15;

    ret = -1;
    EXPECT_DOUBLE_EQ(ret, Round(f, 0));

    ret = -1.2;
    EXPECT_DOUBLE_EQ(ret, Round(f, 1));

    ret = -1.15;
    EXPECT_DOUBLE_EQ(ret, Round(f, 2));

    ret = -1.150;
    EXPECT_DOUBLE_EQ(ret, Round(f, 3));

    // -1.16
    f = -1.16;

    ret = -1;
    EXPECT_DOUBLE_EQ(ret, Round(f, 0));

    ret = -1.2;
    EXPECT_DOUBLE_EQ(ret, Round(f, 1));

    ret = -1.16;
    EXPECT_DOUBLE_EQ(ret, Round(f, 2));

    ret = -1.160;
    EXPECT_DOUBLE_EQ(ret, Round(f, 3));

    // -1.51
    f = -1.51;

    ret = -2;
    EXPECT_DOUBLE_EQ(ret, Round(f, 0));

    ret = -1.5;
    EXPECT_DOUBLE_EQ(ret, Round(f, 1));

    ret = -1.51;
    EXPECT_DOUBLE_EQ(ret, Round(f, 2));

    ret = -1.510;
    EXPECT_DOUBLE_EQ(ret, Round(f, 3));

    // -1.55
    f = -1.55;

    ret = -2;
    EXPECT_DOUBLE_EQ(ret, Round(f, 0));

    ret = -1.6;
    EXPECT_DOUBLE_EQ(ret, Round(f, 1));

    ret = -1.55;
    EXPECT_DOUBLE_EQ(ret, Round(f, 2));

    ret = -1.550;
    EXPECT_DOUBLE_EQ(ret, Round(f, 3));
}

/**
 * @brief 浮点数大小判断测试-f32类型。
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test011()
{
    f32 f1 = 1.1f;
    f32 f2 = 1.2f;
    f32 f3 = 1.10f;

    EXPECT_TRUE(FloatEQ(f1, f3));
    EXPECT_TRUE(FloatGT(f2, f1));
    EXPECT_TRUE(FloatLT(f3, f2));

    f1 = -1.1f;
    f2 = -1.2f;
    f3 = -1.10f;

    EXPECT_TRUE(FloatEQ(f1, f3));
    EXPECT_TRUE(FloatLT(f2, f1));
    EXPECT_TRUE(FloatGT(f3, f2));
}

/**
 * @brief 浮点数大小判断测试-f64类型。
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test012()
{
    f64 f1 = 1.1;
    f64 f2 = 1.2;
    f64 f3 = 1.10;

    EXPECT_TRUE(FloatEQ(f1, f3));
    EXPECT_TRUE(FloatGT(f2, f1));
    EXPECT_TRUE(FloatLT(f3, f2));

    f1 = -1.1;
    f2 = -1.2;
    f3 = -1.10;

    EXPECT_TRUE(FloatEQ(f1, f3));
    EXPECT_TRUE(FloatLT(f2, f1));
    EXPECT_TRUE(FloatGT(f3, f2));
}

/**
 * @brief NOT_LT_MULTI_M测试
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test013()
{
    i32 x = 10;
    i32 m = 2;
    i32 ret = 10;
    EXPECT_EQ(ret, GE_X_MULTI_M(x, m));

    x = 11;
    ret = 12;
    EXPECT_EQ(ret, GE_X_MULTI_M(x, m));

    m = 3;
    EXPECT_EQ(ret, GE_X_MULTI_M(x, m));

    // 已经是16的倍数
    x = 768;
    m = 16;
    EXPECT_EQ(x, GE_X_MULTI_M(x, m));
}

void Test014()
{
    i32 x = 10;
    i32 m = 2;
    i32 ret = 10;
    EXPECT_EQ(ret, LE_X_MULTI_M(x, m));

    x = 11;
    ret = 10;
    EXPECT_EQ(ret, LE_X_MULTI_M(x, m));

    x = 9;
    ret = 8;
    EXPECT_EQ(ret, LE_X_MULTI_M(x, m));

    m = 3;
    ret = 9;
    EXPECT_EQ(ret, LE_X_MULTI_M(x, m));

    // 已经是16的倍数
    x = 768;
    m = 16;
    EXPECT_EQ(x, LE_X_MULTI_M(x, m));
}

ADD_TEST(NumUtilTest, Test001);
ADD_TEST(NumUtilTest, Test002);
ADD_TEST(NumUtilTest, Test003);
ADD_TEST(NumUtilTest, Test004);
ADD_TEST(NumUtilTest, Test005);
ADD_TEST(NumUtilTest, Test006);
ADD_TEST(NumUtilTest, Test007);
ADD_TEST(NumUtilTest, Test008);
ADD_TEST(NumUtilTest, Test009);
ADD_TEST(NumUtilTest, Test010);
ADD_TEST(NumUtilTest, Test011);
ADD_TEST(NumUtilTest, Test012);
ADD_TEST(NumUtilTest, Test013);
ADD_TEST(NumUtilTest, Test014);
}
