#include "test_util.h"
#include "data_type.h"

namespace data_type_test
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
}

ADD_TEST(DataTypeTest, Test001);
} /* namespace data_type_test */
