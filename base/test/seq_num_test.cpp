#include "data_types.h"
#include "seq_num.h"
#include "test_util.h"

#define SEQ_NUM_TEST_NO_TIMEOUT 1

namespace seq_num_test
{
void HashSetTest()
{
    typedef __hash_set<int> MyHashSet;
    MyHashSet x;
    ASSERT_TRUE(x.insert(100).second);
    ASSERT_FALSE(x.insert(100).second); // 已经存在则insert失败
}

void SeqNumExceptionTest()
{
    typedef SeqNum<int, 0, -1> MySeqNum;
    EXPECT_THROW(MySeqNum(), SeqNumException);
}

/**
 * @brief seq测试-分配第1个int32类型的seq
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，分配的seq为1
 *  - Tear Down:
 1，
 * @note
 */
void Test001()
{
    I32SeqNum seq_num;
    i32 n = seq_num.Alloc();
    EXPECT_EQ(seq_num.Min(), n);
}

/**
 * @brief seq测试-分配第(0x7fffffff + 1)个int32类型的seq
 * @details
 *  - Set Up:
 1，已经分配过[1, 0x7fffffff]范围的序列号且都释放了
 *  - Expectation:
 1，分配的seq为1
 *  - Tear Down:
 1，
 * @note
 */
void Test002()
{
    I32SeqNum seq_num;
    i32 n;

    for (i32 i = seq_num.Min(); i <= seq_num.Max(); ++i)
    {
        n = seq_num.Alloc();
        EXPECT_EQ(i, n);
        seq_num.Free(n);
    }

    n = seq_num.Alloc();
    EXPECT_EQ(seq_num.Min(), n);
}

/**
 * @brief seq测试-分配第(0x7fffffff + 1)个int32类型的seq
 * @details
 *  - Set Up:
 1，已经分配过[1, 0x7fffffff]范围的序列号，且[2, 0x7fffffff]都释放了，1还没有释放
 *  - Expectation:
 1，分配的seq为2
 *  - Tear Down:
 1，
 * @note
 */
void Test003()
{
    I32SeqNum seq_num;
    i32 n;

    for (i32 i = seq_num.Min(); i <= seq_num.Max(); ++i)
    {
        n = seq_num.Alloc();
        EXPECT_EQ(i, n);

        if (n != 1)
        {
            seq_num.Free(n);
        }
    }

    n = seq_num.Alloc();
    EXPECT_EQ(seq_num.Min() + 1, n);
}

/**
 * @brief seq测试-分配第1个uint32类型的seq
 * @details
 *  - Set Up:
 1，分配的seq为1
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test004()
{
    U32SeqNum seq_num;
    u32 n = seq_num.Alloc();
    EXPECT_EQ(seq_num.Min(), n);
}

/**
 * @brief seq测试-分配第(0xffffffff + 1)个uint32类型的seq
 * @details
 *  - Set Up:
 1，已经分配过[1, 0xffffffff]范围的序列号且都释放了
 *  - Expectation:
 1，分配的seq为1
 *  - Tear Down:
 1，
 * @note
 */
void Test005()
{
    U32SeqNum seq_num;
    u32 n;

    for (u32 i = seq_num.Min(); i <= seq_num.Max(); ++i)
    {
        n = seq_num.Alloc();
        EXPECT_EQ(i, n);
        seq_num.Free(n);
    }

    n = seq_num.Alloc();
    EXPECT_EQ(seq_num.Min(), n);
}

/**
 * @brief seq测试-分配第(0xffffffff + 1)个uint类型的seq
 * @details
 *  - Set Up:
 1，已经分配过[1, 0xffffffff]范围的序列号，且[2, 0xffffffff]都释放了，1还没有释放
 *  - Expectation:
 1，分配的seq为2
 *  - Tear Down:
 1，
 * @note
 */
void Test006()
{
    U32SeqNum seq_num;
    u32 n;

    for (u32 i = seq_num.Min(); i <= seq_num.Max(); ++i)
    {
        n = seq_num.Alloc();
        EXPECT_EQ(i, n);

        if (n != 1)
        {
            seq_num.Free(n);
        }
    }

    n = seq_num.Alloc();
    EXPECT_EQ(seq_num.Min() + 1, n);
}

// 自定义范围的seq测试
const i32 CUSTOM_SEQ_MIN = 10;
const i32 CUSTOM_SEQ_MAX = 100;
typedef SeqNum<i32, CUSTOM_SEQ_MIN, CUSTOM_SEQ_MAX> MySeq;

/**
 * @brief seq测试-分配第1个int32类型的seq-指定序列号范围
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，分配的seq为MinSeq
 *  - Tear Down:
 1，
 * @note
 */
void Test007()
{
    MySeq seq_num;
    i32 n = seq_num.Alloc();
    EXPECT_EQ(seq_num.Min(), n);
}

/**
 * @brief seq测试-分配第(MaxSeq - MinSeq + 1 + 1)个int32类型的seq-指定序列号范围
 * @details
 *  - Set Up:
 1，已经分配过[MinSeq, MaxSeq]范围的序列号且都释放了。
 *  - Expectation:
 1，分配的seq为MinSeq
 *  - Tear Down:
 1，
 * @note
 */
void Test008()
{
    MySeq seq_num;
    i32 n;

    for (i32 i = seq_num.Min(); i <= seq_num.Max(); ++i)
    {
        n = seq_num.Alloc();
        EXPECT_EQ(i, n);
        seq_num.Free(n);
    }

    n = seq_num.Alloc();
    EXPECT_EQ(seq_num.Min(), n);
}

/**
 * @brief seq测试-分配第(MaxSeq - MinSeq + 1 + 1)个int32类型的seq-指定序列号范围
 * @details
 *  - Set Up:
 1，已经分配过[MinSeq, MaxSeq]范围的序列号，且[MinSeq + 1, MaxSeq]都释放了，MinSeq还没有释放
 *  - Expectation:
 1，分配的seq为(MinSeq + 1)
 *  - Tear Down:
 1，
 * @note
 */
void Test009()
{
    MySeq seq_num;
    i32 n;

    for (i32 i = seq_num.Min(); i <= seq_num.Max(); ++i)
    {
        n = seq_num.Alloc();
        EXPECT_EQ(i, n);

        if (n != CUSTOM_SEQ_MIN)
        {
            seq_num.Free(n);
        }
    }

    n = seq_num.Alloc();
    EXPECT_EQ(seq_num.Min() + 1, n);
}

/**
 * @brief seq测试-分配第(MaxSeq - MinSeq + 1 + 1)个int32类型的seq-指定序列号范围
 * @details
 *  - Set Up:
 1，已经分配过[MinSeq, MaxSeq]范围的序列号，且都未释放
 *  - Expectation:
 1，分配序列号失败，返回值为INVALID_SEQ
 *  - Tear Down:
 1，
 * @note
 */
void Test010()
{
    MySeq seq_num;
    i32 n;

    for (i32 i = seq_num.Min(); i <= seq_num.Max(); ++i)
    {
        n = seq_num.Alloc();
        EXPECT_EQ(i, n);
    }

    n = seq_num.Alloc();
    EXPECT_EQ(INVALID_SEQ_NUM, n);
}

ADD_TEST(SeqNumTest, HashSetTest);
ADD_TEST(SeqNumTest, SeqNumExceptionTest);

#if SEQ_NUM_TEST_NO_TIMEOUT
ADD_TEST(SeqNumTest, Test001);
//ADD_TEST(SeqNumTest, Test002); // timeout
//ADD_TEST(SeqNumTest, Test003); // timeout
ADD_TEST(SeqNumTest, Test004);
//ADD_TEST(SeqNumTest, Test005); // timeout
//ADD_TEST(SeqNumTest, Test006); // timeout
ADD_TEST(SeqNumTest, Test007);
ADD_TEST(SeqNumTest, Test008);
ADD_TEST(SeqNumTest, Test009);
ADD_TEST(SeqNumTest, Test010);
#else
ADD_TEST(SeqNumTest, Test001);
ADD_TEST(SeqNumTest, Test002); // timeout
ADD_TEST(SeqNumTest, Test003); // timeout
ADD_TEST(SeqNumTest, Test004);
ADD_TEST(SeqNumTest, Test005); // timeout
ADD_TEST(SeqNumTest, Test006); // timeout
ADD_TEST(SeqNumTest, Test007);
ADD_TEST(SeqNumTest, Test008);
ADD_TEST(SeqNumTest, Test009);
ADD_TEST(SeqNumTest, Test010);
#endif
} // seq_num_test
