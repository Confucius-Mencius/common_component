#include "backtrace.h"
#include "simple_log.h"
#include "test_util.h"

namespace backtrace_test
{
/**
 * @brief 函数调用堆栈测试-在一层函数调用中打印堆栈。
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 * @note
 */
void Test001()
{
    char buf[4096] = "";
    int ret = Backtrace(buf, sizeof(buf));
    ASSERT_EQ(0, ret);
    LOG_CPP(buf);
}

int Func(int a, int b)
{
    (void) a;
    (void) b;

    char buf[4096] = "";
    int ret = Backtrace(buf, sizeof(buf));
    LOG_CPP(buf);

    return ret;
}

/**
 * @brief 函数调用堆栈测试-在多于一层的函数调用中打印堆栈。
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 * @note
 */
void Test002()
{
    ASSERT_EQ(0, Func(1, 2));
}

class COnly4Test
{
public:
    int Func(int a, int b)
    {
        (void) a;
        (void) b;

        char buf[4096] = "";
        int ret = Backtrace(buf, sizeof(buf));
        LOG_CPP(buf);

        return ret;
    }
};

/**
 * @brief 函数调用堆栈测试-在类的成员函数中打印堆栈。
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 * @note
 */
void Test003()
{
    COnly4Test x;
    ASSERT_EQ(0, x.Func(1, 2));
}

/**
 * @brief 函数调用堆栈测试-静态函数不会显示在调用堆栈中。
 * @details
 *  - Set Up:
 1，Test004是一个静态函数。
 2，在Test004中获取函数调用堆栈。
 *  - Expectation:
 1，调用堆栈中没有Test004这个静态函数。
 *  - Tear Down:
 * @note
 */
static void Test004()
{
    char buf[4096] = "";
    int ret = Backtrace(buf, sizeof(buf));
    ASSERT_EQ(0, ret);
    LOG_CPP(buf);
}

ADD_TEST(BacktraceTest, Test001);
ADD_TEST(BacktraceTest, Test002);
ADD_TEST(BacktraceTest, Test003);
ADD_TEST(BacktraceTest, Test004);
} // namespace backtrace_test

