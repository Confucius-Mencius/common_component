#include "test_util.h"
#include "variant.h"

namespace variant_test
{
/**
 * @brief Variant测试-i32类型测试
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
    i32 val = 1;

    // 使用构造函数赋值
    Variant v1(val);
    EXPECT_TRUE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_EQ(val, v1.GetValue(Type2Type<i32>()));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_F64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_STR));

    // 使用赋值运算符赋值
    Variant v2;
    v2 = val;
    EXPECT_TRUE(v2.TypeMatch(Variant::TYPE_I32));
    EXPECT_EQ(val, v2.GetValue(Type2Type<i32>()));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v2.TypeMatch(Variant::TYPE_F64));
    EXPECT_FALSE(v2.TypeMatch(Variant::TYPE_STR));

    // 类型操作
    Variant v3;
    v3.SetType(Variant::TYPE_I32);
    EXPECT_TRUE(v3.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v3.TypeMatch(Variant::TYPE_F64));
    EXPECT_FALSE(v3.TypeMatch(Variant::TYPE_STR));

    v3 = val;
    EXPECT_TRUE(v3.TypeMatch(Variant::TYPE_I32));
    EXPECT_EQ(val, v3.GetValue(Type2Type<i32>()));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v3.TypeMatch(Variant::TYPE_F64));
    EXPECT_FALSE(v3.TypeMatch(Variant::TYPE_STR));
}

/**
     * @brief Variant测试-i64类型测试
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
    i64 val = 0xffffffff;

    // 使用构造函数赋值
    Variant v1(val);
    EXPECT_TRUE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_EQ(val, v1.GetValue(Type2Type<i64>()));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_F64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_STR));

    // 使用赋值运算符赋值
    Variant v2;
    v2 = val;
    EXPECT_TRUE(v2.TypeMatch(Variant::TYPE_I64));
    EXPECT_EQ(val, v2.GetValue(Type2Type<i64>()));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_F64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_STR));

    // 类型操作
    Variant v3;
    v3.SetType(Variant::TYPE_I64);
    EXPECT_TRUE(v3.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_F64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_STR));

    v3 = val;
    EXPECT_TRUE(v3.TypeMatch(Variant::TYPE_I64));
    EXPECT_EQ(val, v3.GetValue(Type2Type<i64>()));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_F64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_STR));
}

/**
 * @brief Variant测试-f64类型测试
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
    f64 val = 1.0;

    // 使用构造函数赋值
    Variant v1(val);
    EXPECT_TRUE(v1.TypeMatch(Variant::TYPE_F64));
    EXPECT_DOUBLE_EQ(val, v1.GetValue(Type2Type<f64>()));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_STR));

    // 使用赋值运算符赋值
    Variant v2;
    v2 = val;
    EXPECT_TRUE(v2.TypeMatch(Variant::TYPE_F64));
    EXPECT_DOUBLE_EQ(val, v2.GetValue(Type2Type<f64>()));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_STR));

    // 类型操作
    Variant v3;
    v3.SetType(Variant::TYPE_F64);
    EXPECT_TRUE(v3.TypeMatch(Variant::TYPE_F64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_STR));

    v3 = val;
    EXPECT_TRUE(v3.TypeMatch(Variant::TYPE_F64));
    EXPECT_DOUBLE_EQ(val, v3.GetValue(Type2Type<f64>()));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_STR));
}

/**
 * @brief Variant测试-字符串类型测试
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
    const char val[] = "hello, world!";

    // 使用构造函数赋值
    Variant v1(Variant::TYPE_STR, val, strlen(val));
    EXPECT_TRUE(v1.TypeMatch(Variant::TYPE_STR));

    const Variant::Chunk& chunk = v1.GetValue(Type2Type<const char*>());
    EXPECT_STREQ(val, chunk.data);

    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_F64));

    // 使用赋值运算符赋值
    Variant v2;
    v2 = std::pair<Variant::Type, Variant::Chunk>(Variant::TYPE_STR, chunk);
    EXPECT_TRUE(v2.TypeMatch(Variant::TYPE_STR));

    const Variant::Chunk& chunk2 = v2.GetValue(Type2Type<const char*>());
    EXPECT_STREQ(val, chunk2.data);

    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_F64));

    // 类型操作
    Variant v3;
    v3.SetType(Variant::TYPE_STR);
    EXPECT_TRUE(v3.TypeMatch(Variant::TYPE_STR));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_F64));

    v3 = std::pair<Variant::Type, Variant::Chunk>(Variant::TYPE_STR, chunk);
    EXPECT_TRUE(v3.TypeMatch(Variant::TYPE_STR));

    const Variant::Chunk& chunk3 = v3.GetValue(Type2Type<const char*>());
    EXPECT_STREQ(val, chunk3.data);
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I32));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_I64));
    EXPECT_FALSE(v1.TypeMatch(Variant::TYPE_F64));
}

/**
 * @brief ToString测试
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void Test005()
{
    Variant v(9999);
    EXPECT_EQ(Variant::TYPE_I32, v.GetType());

    char buf[8] = "";
    v.ToString(buf, sizeof(buf));
    std::cout << buf << std::endl;

    v = INT64_C(9999);
    EXPECT_EQ(Variant::TYPE_I64, v.GetType());
    v.ToString(buf, sizeof(buf));
    std::cout << buf << std::endl;

    v = 9999.10;
    EXPECT_EQ(Variant::TYPE_F64, v.GetType());
    v.ToString(buf, sizeof(buf));
    std::cout << buf << std::endl;

    std::pair<Variant::Type, Variant::Chunk> p;
    p.first = Variant::TYPE_STR;
    p.second.data = (char*) "9999";
    p.second.len = strlen("9999");
    v = p;
    EXPECT_EQ(Variant::TYPE_STR, v.GetType());
    v.ToString(buf, sizeof(buf));
    std::cout << buf << std::endl;
}

void Test006()
{
    std::map<int, Variant> m1;

    for (int i = 0; i < 1; ++i)
    {
        m1[1] = Variant(Variant::TYPE_STR, "XX", 2);
    }

    m1[1] = Variant(Variant::TYPE_STR, "YYY", 3);
}

ADD_TEST(VariantTest, Test001);
ADD_TEST(VariantTest, Test002);
ADD_TEST(VariantTest, Test003);
ADD_TEST(VariantTest, Test004);
ADD_TEST(VariantTest, Test005);
ADD_TEST(VariantTest, Test006);
} /* namespace variant_test */
