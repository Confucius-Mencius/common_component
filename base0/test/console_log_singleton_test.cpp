#include "console_log_singleton_test.h"
#include "console_log_singleton.h"

ConsoleLogSingletonTest::ConsoleLogSingletonTest()
{

}

ConsoleLogSingletonTest::~ConsoleLogSingletonTest()
{

}

void ConsoleLogSingletonTest::Test001()
{
    LOG_TRACE("hello world");
    LOG_TRACE("hello " << 1);

    LOG_DEBUG("hello world");
    LOG_DEBUG("hello " << 1);

    LOG_INFO("hello world");
    LOG_INFO("hello " << 1);

    LOG_WARN("hello world");
    LOG_WARN("hello " << 1);

    LOG_ERROR("hello world");
    LOG_ERROR("hello " << 1);

    LOG_FATAL("hello world");
    LOG_FATAL("hello " << 1);
}

ADD_TEST_F(ConsoleLogSingletonTest, Test001);
