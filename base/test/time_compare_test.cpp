#include "time_compare.h"
#include "data_types.h"
#include "test_util.h"
#include "simple_log.h"

namespace time_compare_test
{
void Test001()
{
    unsigned long l;
    TYPE_CHECK(unsigned long, l);
//    TYPE_CHECK(long, l); // 编不过
}

// 以u8和i8类型为例验证
void Test002()
{
    {
//        TIME_AFTER(b, a);
        u8 a = U8_MIN, b = U8_MAX - 1;
        LOG_CPP("a\tb\td");

        while (true)
        {
            --b;
            i8 d = (i8) a - (i8) b;
            LOG_CPP((int)a << "\t" << (int)b << "\t" << (int)d);
            if (d < 0)
            {
                LOG_CPP("I8_MAX: " << I8_MAX);
                break; // 0 128 -128  此时a和b之间有127个空位，是临界状态了
            }
        }
    }

    {
        //        TIME_AFTER(a, b);
        u8 a = U8_MAX - 2, b = U8_MAX - 1;
        LOG_CPP("a\tb\td");

        while (true)
        {
            ++a;
            i8 d = (i8) b - (i8) a;
            LOG_CPP((int)a << "\t" << (int)b << "\t" << (int)d);
            if (d > 0)
            {
                LOG_CPP("I8_MAX: " << I8_MAX);
                break; // 126   254 -128  此时254后面有1个空位，然后0-125一共是126个空位，加起来时127个空位，也是临界状态了
            }
        }
    }
}

void Test003()
{
    unsigned long t0 = ULONG_MIN;
    LOG_CPP(t0);
    unsigned long t1 = ULONG_MAX - 1;
    unsigned long t2 = ULONG_MAX;
    unsigned long t3 = ULONG_MAX + 1;
    LOG_CPP(t3);

    long d1 = (long)t0 - (long)t1;
    LOG_CPP(d1); // 2

    long d2 = (long)t1 - (long)t2;
    LOG_CPP(d2); // -1

    long d3 = (long)t2 - (long)t3;
    LOG_CPP(d3); // -1

    EXPECT_FALSE(TIME_AFTER(t1, t0)); // t1和t0之间的空位远大于LONG_MAX，不符合要求
    EXPECT_TRUE(TIME_AFTER(t2, t1));
    EXPECT_TRUE(TIME_AFTER(t3, t2));
}

void Test004()
{
    unsigned long t0 = ULONG_MIN;
    unsigned long t1 = ULONG_MIN + LONG_MAX; // t1和t0之间有LONG_MAX-1个空位，符合要求
    unsigned long t2 = ULONG_MIN + LONG_MAX + 1; // t2和t0之间有LONG_MAX个空位，符合要求
    unsigned long t3 = ULONG_MIN + LONG_MAX + 2; // t3和t0之间有LONG_MAX+1个空位，不符合要求

    long d1 = (long)t0 - (long)t1;
    LOG_CPP(d1); //

    long d2 = (long)t0 - (long)t2;
    LOG_CPP(d2); //

    long d3 = (long)t0 - (long)t3;
    LOG_CPP(d3); //

    EXPECT_TRUE(TIME_AFTER(t1, t0));
    EXPECT_TRUE(TIME_AFTER(t2, t0));
    EXPECT_FALSE(TIME_AFTER(t3, t0));
}

ADD_TEST(TimeCompareTest, Test001);
ADD_TEST(TimeCompareTest, Test002);
ADD_TEST(TimeCompareTest, Test003);
ADD_TEST(TimeCompareTest, Test004);
}
