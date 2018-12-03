#ifndef BASE_INC_DEMO_TEST_H_
#define BASE_INC_DEMO_TEST_H_

#include "test_util.h"
#include "simple_log.h"

class DemoTest : public GTest
{
public:
    static void SetUpTestCase()
    {
        LOG_CPP("该函数在本组（DemoTest类）的所有用例之前执行");
    }

    static void TearDownTestCase()
    {
        LOG_CPP("该函数在本组（DemoTest类）的所有用例之后执行");
    }

    virtual void SetUp()
    {
        LOG_CPP("该函数在本组内的每个用例之前执行");
    }

    virtual void TearDown()
    {
        LOG_CPP("该函数在本组内的每个用例之后执行");
    }

    void Test001()
    {
        LOG_CPP("test 1");
    }

    void Test002()
    {
        LOG_CPP("test 2");
    }
};

#endif // BASE_INC_DEMO_TEST_H_
