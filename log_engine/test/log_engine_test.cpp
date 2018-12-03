#include "log_engine_test.h"
#include "console_log_engine.h"
#include "log_util.h"
#include "mem_util.h"

static const char log_conf_file_path[] = "./test_log_conf.properties";
static const char logger_name[] = "my_server";

LogEngineInterface* g_log_engine;

LogEngineTest::LogEngineTest() : loader_()
{
    log_engine_ = NULL;
}

LogEngineTest::~LogEngineTest()
{
}

void LogEngineTest::SetUp()
{
    if (loader_.Load("../liblog_engine.so") != 0)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    log_engine_ = (LogEngineInterface*) loader_.GetModuleInterface(0);
    if (NULL == log_engine_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    std::cout << "log engine version: " << log_engine_->GetVersion() << std::endl;

    LogEngineCtx log_engine_ctx;
    log_engine_ctx.log_conf_file_path = log_conf_file_path;
    log_engine_ctx.logger_name = logger_name;

    if (log_engine_->Initialize(&log_engine_ctx) != 0)
    {
        FAIL() << log_engine_->GetLastErrMsg();
    }

    g_log_engine = log_engine_;

    if (log_engine_->Activate() != 0)
    {
        FAIL() << log_engine_->GetLastErrMsg();
    }
}

void LogEngineTest::TearDown()
{
    SAFE_DESTROY_MODULE(log_engine_, loader_);
}

void LogEngineTest::Test001()
{
    LOG_TRACE("hello");
    LOG_DEBUG("hello");
    LOG_WARN("hello");
    LOG_ERROR("hello");
    LOG_FATAL("hello");
}

void LogEngineTest::Test002()
{
    LOG_TRACE("hello");
    LOG_DEBUG("hello");
    LOG_WARN("hello");
    LOG_ERROR("hello");
    LOG_FATAL("hello");

    // 修改日志级别
    log_engine_->Reload();

    LOG_TRACE("hello");
    LOG_DEBUG("hello");
    LOG_WARN("hello");
    LOG_ERROR("hello");
    LOG_FATAL("hello");
    LOG_ALWAYS("hello");
}

void LogEngineTest::Test003()
{
    // 主线程，不停地修改日志级别
    // 子线程，不停地写日志
}

void LogEngineTest::ConsoleLogEngineTest()
{
    ConsoleLogEngine console_log_engine;
    g_log_engine = &console_log_engine;
    LOG_TRACE("console log engine");
    LOG_ALWAYS("console log engine");
}

ADD_TEST_F(LogEngineTest, Test001);
ADD_TEST_F(LogEngineTest, Test002);
ADD_TEST_F(LogEngineTest, Test003);
ADD_TEST_F(LogEngineTest, ConsoleLogEngineTest);
