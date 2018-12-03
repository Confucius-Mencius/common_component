#include "test_util.h"
#include "simple_array.h"

namespace simple_array_test
{
static const int capacity = 10;
typedef SimpleArray<int, capacity> MyArray;

void Test001()
{
    MyArray array;

    EXPECT_EQ(0, array.Size());
    EXPECT_EQ(NULL, array.GetElem(0));
    EXPECT_EQ(NULL, array.GetElem(capacity - 1));
    EXPECT_EQ(NULL, array.GetElem(-1));
    EXPECT_EQ(NULL, array.GetElem(capacity));
    EXPECT_TRUE(array.IsEmpty());
    EXPECT_FALSE(array.IsFull());

    for (int i = 0; i < capacity; ++i)
    {
        EXPECT_EQ(i, array.Size());
        EXPECT_FALSE(array.IsFull());

        array.AppendElem(i);

        EXPECT_EQ(i + 1, array.Size());

        for (int j = 0; j <= i; ++j)
        {
            EXPECT_TRUE(array.GetElem(j) != NULL);
        }

        for (int k = i + 1; k < capacity; ++k)
        {
            EXPECT_TRUE(NULL == array.GetElem(k));
        }
    }

    EXPECT_EQ(capacity, array.Size());
    for (int i = 0; i < capacity; ++i)
    {
        EXPECT_TRUE(array.GetElem(i) != NULL);
    }

    EXPECT_TRUE(array.IsFull());
    EXPECT_FALSE(array.IsEmpty());
}

ADD_TEST(SimpleArrayTest, Test001);
} /* namespace simple_array_test */
