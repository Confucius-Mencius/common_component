#include "spd_log_util_test.h"
#include <thread>
#include "spd_log_util.h"
#include "str_util.h"

namespace spd_log_util_test
{
SPDLogUtilTest::SPDLogUtilTest()
{
}

SPDLogUtilTest::~SPDLogUtilTest()
{
}

void SPDLogUtilTest::SetUpTestCase()
{
    SpdLogCtx ctx;
    ctx.logger_file_path = "./logs/spdlog.log";
    ctx.logger_name = "spdlog";

    EXPECT_EQ(0, SpdLogInitialize(&ctx));
}

void SPDLogUtilTest::TearDownTestCase()
{
    SpdLogFinalize();
}

void SPDLogUtilTest::Test001()
{
//    https://github.com/fmtlib/fmt

    // 使用占位符
    SPD_LOG_TRACE("spdlog", "Hello {}", "World");
    // 带格式化的占位符：d整数，x十六进制，o八进制，b二进制
    SPD_LOG_TRACE("spdlog", "Support for int: {0:d}; hex: {0:x}; oct: {0:o}; bin: {0:b}", 15);
    // 带格式化的占位符：f浮点数
    SPD_LOG_TRACE("spdlog", "Support for floats {:03.2f}", 1.23456);
    // 左对齐，保证30字符宽度
    SPD_LOG_TRACE("spdlog", "{:<30}", "left aligned");
    // 右对齐，保证30字符宽度
    SPD_LOG_TRACE("spdlog", "{:>30}", "right aligned");
    // 指定占位符位置序号
    SPD_LOG_TRACE("spdlog", "Positional args are {1} {0}..", "too", "supported");
}

void SPDLogUtilTest::Test002()
{
    SPD_LOG_TRACE("spdlog", "hello {}, {}, pid: {}, tid: {}", "world", 10000, getpid(), pthread_self()); // spdlog打印的线程id不对
    SPD_LOG_DEBUG("spdlog", "hello {}, {}, pid: {}, tid: {}", "world", 10000, getpid(), pthread_self());
    SPD_LOG_INFO("spdlog", "hello {}, {}, pid: {}, tid: {}", "world", 10000, getpid(), pthread_self());
    SPD_LOG_WARN("spdlog", "hello {}, {}, pid: {}, tid: {}", "world", 10000, getpid(), pthread_self());
    SPD_LOG_ERROR("spdlog", "hello {}, {}, pid: {}, tid: {}", "world", 10000, getpid(), pthread_self());
}

void SPDLogUtilTest::Test003()
{
    std::thread t([&]()
    {
        SPD_LOG_TRACE("spdlog", "hello {}, {}, pid: {}, tid: {}", "world", 10000, getpid(), pthread_self());
    });

    t.join();
}

// 动态调整日志级别
void SPDLogUtilTest::Test004()
{
    SPD_LOG_TRACE("spdlog", "1");
    SPD_LOG_DEBUG("spdlog", "1");

    SpdLogSetLevel(spdlog::level::debug);

    SPD_LOG_TRACE("spdlog", "1");
    SPD_LOG_DEBUG("spdlog", "1");

    SpdLogSetLevel(spdlog::level::info);

    SPD_LOG_TRACE("spdlog", "1");
    SPD_LOG_DEBUG("spdlog", "1");
}

ADD_TEST_F(SPDLogUtilTest, Test001);
ADD_TEST_F(SPDLogUtilTest, Test002);
ADD_TEST_F(SPDLogUtilTest, Test003);
ADD_TEST_F(SPDLogUtilTest, Test004);
}
