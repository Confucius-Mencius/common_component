#include "test_util.h"
#include "num_util.h"

static i32 Distance(i32 x1, i32 y1, i32 x2, i32 y2)
{
    const i64 h = ((i64) x2) - ((i64) x1);
    const i64 v = ((i64) y2) - ((i64) y1);
    const i64 r = h * h + v * v;

    assert(r < F32_MAX);
    const i64 ret = (i64) Round(QuickReciprocalSQRT((float) r) * r, 0);

    assert(ret <= I32_MAX);
    return (i32) ret;
}

static bool DistanceGT(i32 x1, i32 y1, i32 x2, i32 y2, i32 threshold)
{
    const i64 h = ((i64) x2) - ((i64) x1);
    const i64 v = ((i64) y2) - ((i64) y1);
    const i64 r = h * h + v * v;
    const i64 t = threshold;

    return ((r > (t * t)) ? true : false);
}

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
static void Test001()
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
static void Test002()
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
static void Test003()
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
static void Test004()
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
static void Test005()
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
static void Test006()
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
static void Test007()
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
static void Test008()
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
static void Test009()
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
static void Test010()
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
static void Test011()
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
static void Test012()
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
 * @brief 安全加测试-i32类型
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
    // 不会超过逻辑上限
    i32 cur_val = 100;
    i32 delta = 50;
    i32 limit = 151;
    bool exceed = false;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, I32_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, I32_MAX, exceed));

    cur_val = 100;
    delta = 50;
    limit = 151;
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, I32_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, I32_MAX, exceed));

    // 会超过逻辑上限
    cur_val = 100;
    delta = 50;
    limit = 149;
    exceed = false;
    EXPECT_FALSE(CanPlus(cur_val, delta, limit, I32_MAX, exceed));
    EXPECT_EQ(cur_val, Plus(cur_val, delta, limit, I32_MAX, exceed));

    cur_val = 100;
    delta = 50;
    limit = 149;
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, I32_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, I32_MAX, exceed));

    cur_val = I32_MAX - 1;
    delta = 50;
    limit = 149;
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, I32_MAX, exceed));
    EXPECT_EQ(I32_MAX, Plus(cur_val, delta, limit, I32_MAX, exceed));

    // cur_val为负数，limit为正数
    cur_val = -100;
    delta = I32_MAX;
    limit = I32_MAX - 101;
    exceed = false;
    EXPECT_FALSE(CanPlus(cur_val, delta, limit, I32_MAX, exceed));
    EXPECT_EQ(cur_val, Plus(cur_val, delta, limit, I32_MAX, exceed));

    cur_val = -100;
    delta = I32_MAX;
    limit = I32_MAX - 101;
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, I32_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, I32_MAX, exceed));
}

/**
 * @brief 安全加测试-u32类型
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
    // 不会超过逻辑上限
    u32 cur_val = UINT32_C(100);
    u32 delta = UINT32_C(50);
    u32 limit = UINT32_C(151);
    bool exceed = false;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, U32_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, U32_MAX, exceed));

    cur_val = UINT32_C(100);
    delta = UINT32_C(50);
    limit = UINT32_C(151);
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, U32_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, U32_MAX, exceed));

    // 会超过逻辑上限
    cur_val = UINT32_C(100);
    delta = UINT32_C(50);
    limit = UINT32_C(149);
    exceed = false;
    EXPECT_FALSE(CanPlus(cur_val, delta, limit, U32_MAX, exceed));
    EXPECT_EQ(cur_val, Plus(cur_val, delta, limit, U32_MAX, exceed));

    cur_val = UINT32_C(100);
    delta = UINT32_C(50);
    limit = UINT32_C(149);
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, U32_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, U32_MAX, exceed));

    cur_val = U32_MAX - 1;
    delta = UINT32_C(50);
    limit = UINT32_C(149);
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, U32_MAX, exceed));
    EXPECT_EQ(U32_MAX, Plus(cur_val, delta, limit, U32_MAX, exceed));
}

/**
 * @brief 安全加测试-i64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test015()
{
    // 不会超过逻辑上限
    i64 cur_val = INT64_C(100);
    i64 delta = INT64_C(50);
    i64 limit = INT64_C(151);
    bool exceed = false;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, I64_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, I64_MAX, exceed));

    cur_val = INT64_C(100);
    delta = INT64_C(50);
    limit = INT64_C(151);
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, I64_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, I64_MAX, exceed));

    // 会超过逻辑上限
    cur_val = INT64_C(100);
    delta = INT64_C(50);
    limit = INT64_C(149);
    exceed = false;
    EXPECT_FALSE(CanPlus(cur_val, delta, limit, I64_MAX, exceed));
    EXPECT_EQ(cur_val, Plus(cur_val, delta, limit, I64_MAX, exceed));

    cur_val = INT64_C(100);
    delta = INT64_C(50);
    limit = INT64_C(149);
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, I64_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, I64_MAX, exceed));

    cur_val = I64_MAX - 1;
    delta = INT64_C(50);
    limit = INT64_C(149);
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, I64_MAX, exceed));
    EXPECT_EQ(I64_MAX, Plus(cur_val, delta, limit, I64_MAX, exceed));

    // cur_val为负数，limit为正数
    cur_val = -100;
    delta = I64_MAX;
    limit = I64_MAX - 101;
    exceed = false;
    EXPECT_FALSE(CanPlus(cur_val, delta, limit, I64_MAX, exceed));
    EXPECT_EQ(cur_val, Plus(cur_val, delta, limit, I64_MAX, exceed));

    cur_val = -100;
    delta = I64_MAX;
    limit = I64_MAX - 101;
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, I64_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, I64_MAX, exceed));
}

/**
 * @brief 安全加测试-u64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test016()
{
    // 不会超过逻辑上限
    u64 cur_val = UINT64_C(100);
    u64 delta = UINT64_C(50);
    u64 limit = UINT64_C(151);
    bool exceed = false;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, U64_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, U64_MAX, exceed));

    cur_val = UINT64_C(100);
    delta = UINT64_C(50);
    limit = UINT64_C(151);
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, U64_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, U64_MAX, exceed));

    // 会超过逻辑上限
    cur_val = UINT64_C(100);
    delta = UINT64_C(50);
    limit = UINT64_C(149);
    exceed = false;
    EXPECT_FALSE(CanPlus(cur_val, delta, limit, U64_MAX, exceed));
    EXPECT_EQ(cur_val, Plus(cur_val, delta, limit, U64_MAX, exceed));

    cur_val = UINT64_C(100);
    delta = UINT64_C(50);
    limit = UINT64_C(149);
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, U64_MAX, exceed));
    EXPECT_EQ(cur_val + delta, Plus(cur_val, delta, limit, U64_MAX, exceed));

    cur_val = U64_MAX - 1;
    delta = UINT64_C(50);
    limit = UINT64_C(149);
    exceed = true;
    EXPECT_TRUE(CanPlus(cur_val, delta, limit, U64_MAX, exceed));
    EXPECT_EQ(U64_MAX, Plus(cur_val, delta, limit, U64_MAX, exceed));
}

/**
 * @brief 安全加测试-f32类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test017()
{
//    FAIL()<< "f32 plus test";
}

/**
 * @brief 安全加测试-f64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test018()
{
//    FAIL()<< "f64 plus test";
}

/**
 * @brief 安全减测试-i32类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test019()
{
    // 不会超过逻辑下限
    i32 cur_val = 100;
    i32 delta = 50;
    i32 limit = 49;
    bool exceed = false;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, I32_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, I32_MIN, exceed));

    cur_val = 100;
    delta = 50;
    limit = 49;
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, I32_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, I32_MIN, exceed));

    // 会超过逻辑下限
    cur_val = 100;
    delta = 50;
    limit = 51;
    exceed = false;
    EXPECT_FALSE(CanMinus(cur_val, delta, limit, I32_MIN, exceed));
    EXPECT_EQ(cur_val, Minus(cur_val, delta, limit, I32_MIN, exceed));

    cur_val = 100;
    delta = 50;
    limit = 51;
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, I32_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, I32_MIN, exceed));

    cur_val = I32_MIN + 1;
    delta = 50;
    limit = 51;
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, I32_MIN, exceed));
    EXPECT_EQ(I32_MIN, Minus(cur_val, delta, limit, I32_MIN, exceed));

    // cur_val为负数，limit为正数
    cur_val = 100;
    delta = I32_MAX;
    limit = I32_MIN + 102;
    exceed = false;
    EXPECT_FALSE(CanMinus(cur_val, delta, limit, I32_MIN, exceed));
    EXPECT_EQ(cur_val, Minus(cur_val, delta, limit, I32_MIN, exceed));

    cur_val = 100;
    delta = I32_MAX;
    limit = I32_MIN + 102;
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, I32_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, I32_MIN, exceed));
}

/**
 * @brief 安全减测试-u32类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test020()
{
    // 不会超过逻辑下限
    u32 cur_val = UINT32_C(100);
    u32 delta = UINT32_C(50);
    u32 limit = UINT32_C(49);
    bool exceed = false;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, U32_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, U32_MIN, exceed));

    cur_val = UINT32_C(100);
    delta = UINT32_C(50);
    limit = UINT32_C(49);
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, U32_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, U32_MIN, exceed));

    // 会超过逻辑下限
    cur_val = UINT32_C(100);
    delta = UINT32_C(50);
    limit = UINT32_C(51);
    exceed = false;
    EXPECT_FALSE(CanMinus(cur_val, delta, limit, U32_MIN, exceed));
    EXPECT_EQ(cur_val, Minus(cur_val, delta, limit, U32_MIN, exceed));

    cur_val = UINT32_C(100);
    delta = UINT32_C(50);
    limit = UINT32_C(51);
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, U32_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, U32_MIN, exceed));

    cur_val = U32_MIN + 1;
    delta = UINT32_C(50);
    limit = UINT32_C(51);
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, U32_MIN, exceed));
    EXPECT_EQ(U32_MIN, Minus(cur_val, delta, limit, U32_MIN, exceed));
}

/**
 * @brief 安全减测试-i64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test021()
{
    // 不会超过逻辑下限
    i64 cur_val = INT64_C(100);
    i64 delta = INT64_C(50);
    i64 limit = INT64_C(49);
    bool exceed = false;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, I64_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, I64_MIN, exceed));

    cur_val = INT64_C(100);
    delta = INT64_C(50);
    limit = INT64_C(49);
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, I64_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, I64_MIN, exceed));

    // 会超过逻辑下限
    cur_val = INT64_C(100);
    delta = INT64_C(50);
    limit = INT64_C(51);
    exceed = false;
    EXPECT_FALSE(CanMinus(cur_val, delta, limit, I64_MIN, exceed));
    EXPECT_EQ(cur_val, Minus(cur_val, delta, limit, I64_MIN, exceed));

    cur_val = INT64_C(100);
    delta = INT64_C(50);
    limit = INT64_C(51);
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, I64_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, I64_MIN, exceed));

    cur_val = I64_MIN + 1;
    delta = INT64_C(50);
    limit = INT64_C(51);
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, I64_MIN, exceed));
    EXPECT_EQ(I64_MIN, Minus(cur_val, delta, limit, I64_MIN, exceed));
}

/**
 * @brief 安全减测试-u64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test022()
{
    // 不会超过逻辑下限
    u64 cur_val = UINT64_C(100);
    u64 delta = UINT64_C(50);
    u64 limit = UINT64_C(49);
    bool exceed = false;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, U64_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, U64_MIN, exceed));

    cur_val = UINT64_C(100);
    delta = UINT64_C(50);
    limit = UINT64_C(49);
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, U64_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, U64_MIN, exceed));

    // 会超过逻辑下限
    cur_val = UINT64_C(100);
    delta = UINT64_C(50);
    limit = UINT64_C(51);
    exceed = false;
    EXPECT_FALSE(CanMinus(cur_val, delta, limit, U64_MIN, exceed));
    EXPECT_EQ(cur_val, Minus(cur_val, delta, limit, U64_MIN, exceed));

    cur_val = UINT64_C(100);
    delta = UINT64_C(50);
    limit = UINT64_C(51);
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, U64_MIN, exceed));
    EXPECT_EQ(cur_val - delta, Minus(cur_val, delta, limit, U64_MIN, exceed));

    cur_val = U64_MIN + 1;
    delta = UINT64_C(50);
    limit = UINT64_C(51);
    exceed = true;
    EXPECT_TRUE(CanMinus(cur_val, delta, limit, U64_MIN, exceed));
    EXPECT_EQ(U64_MIN, Minus(cur_val, delta, limit, U64_MIN, exceed));
}

/**
 * @brief 安全减测试-f32类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test023()
{
//    FAIL()<< "f32 minus test";
}

/**
 * @brief 安全减测试-f64类型
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test024()
{
//    FAIL()<< "f64 minus test";
}

/**
 * @brief 两个i32类型的正数相乘测试
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test025()
{
    // 结果不会溢出
    i32 a = 100;
    i32 b = 200;
    i32 i;
    i32 ret;

    i = 100 * 200;
    EXPECT_TRUE(Multiply(ret, a, b));
    EXPECT_EQ(i, ret);

    // 结果会溢出
    a = I32_MAX - 1;
    EXPECT_FALSE(Multiply(ret, a, b));

    // 参数错误
    a = -1;
    EXPECT_FALSE(Multiply(ret, a, b));

    // 参数错误
    a = 0;
    EXPECT_FALSE(Multiply(ret, a, b));
}

/**
 * @brief 平方根的倒数测试
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test026()
{
    i32 x = 4;
    f32 ret = 0.49915358f; //0.5f;
    EXPECT_FLOAT_EQ(ret, QuickReciprocalSQRT(x));
    //EXPECT_FLOAT_EQ(ret, Q_RSQRT(x)); // 两种实现一样
}

/**
 * @brief 两点间的距离测试
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
static void Test027()
{
    i32 x1 = 0;
    i32 y1 = 0;
    i32 x2 = 3;
    i32 y2 = 4;
    i32 d = 5;
    EXPECT_EQ(d, Distance(x1, y1, x2, y2));
    EXPECT_TRUE(DistanceGT(x1, y1, x2, y2, 4));
    EXPECT_FALSE(DistanceGT(x1, y1, x2, y2, 6));
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
static void Test028()
{
    i32 x = 10;
    i32 m = 2;
    i32 ret = 10;
    EXPECT_EQ(ret, NOT_LT_MULTI_M(x, m));

    x = 11;
    ret = 12;
    EXPECT_EQ(ret, NOT_LT_MULTI_M(x, m));

    m = 3;
    EXPECT_EQ(ret, NOT_LT_MULTI_M(x, m));
}

static void Test029()
{
    i32 x = 10;
    i32 m = 2;
    i32 ret = 10;
    EXPECT_EQ(ret, NOT_GT_MULTI_M(x, m));

    x = 11;
    ret = 10;
    EXPECT_EQ(ret, NOT_GT_MULTI_M(x, m));

    x = 9;
    ret = 8;
    EXPECT_EQ(ret, NOT_GT_MULTI_M(x, m));

    m = 3;
    ret = 9;

    EXPECT_EQ(ret, NOT_GT_MULTI_M(x, m));
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
ADD_TEST(NumUtilTest, Test015);
ADD_TEST(NumUtilTest, Test016);
ADD_TEST(NumUtilTest, Test017);
ADD_TEST(NumUtilTest, Test018);
ADD_TEST(NumUtilTest, Test019);
ADD_TEST(NumUtilTest, Test020);
ADD_TEST(NumUtilTest, Test021);
ADD_TEST(NumUtilTest, Test022);
ADD_TEST(NumUtilTest, Test023);
ADD_TEST(NumUtilTest, Test024);
ADD_TEST(NumUtilTest, Test025);
ADD_TEST(NumUtilTest, Test026);
ADD_TEST(NumUtilTest, Test027);
ADD_TEST(NumUtilTest, Test028);
ADD_TEST(NumUtilTest, Test029);

namespace int_min_test
{
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
static void Test001()
{
    //EXPECT_TRUE(-2147483648 > 0); // linux下编译报错
    EXPECT_TRUE(-2147483647 - 1 < 0);
    //EXPECT_TRUE(INT_MIN == -INT_MIN); // linux下编译报错，因为-INT_MIN已经超出了I32_MAX
    EXPECT_TRUE(FLT_MIN > 0);
}

ADD_TEST(IntMinTest, Test001);
} // namespace int_min_test