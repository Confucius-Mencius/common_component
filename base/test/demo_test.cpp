#include "demo_test.h"
#include "simple_log.h"

namespace demo_test
{
void DemoTest::SetUpTestCase()
{
    LOG_CPP("该函数在本组（DemoTest类）的所有用例之前执行");
}

void DemoTest::TearDownTestCase()
{
    LOG_CPP("该函数在本组（DemoTest类）的所有用例之后执行");
}

void DemoTest::SetUp()
{
    LOG_CPP("该函数在本组内的每个用例之前执行");
}

void DemoTest::TearDown()
{
    LOG_CPP("该函数在本组内的每个用例之后执行");
}

void DemoTest::Test001()
{
    LOG_CPP("test 1");
}

void DemoTest::Test002()
{
    LOG_CPP("test 2");
}

ADD_TEST_F(DemoTest, Test001);
ADD_TEST_F(DemoTest, Test002);
}
