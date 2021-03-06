#include "log_engine_test.h"
#include "console_log_engine.h"
#include "log_util.h"
#include "mem_util.h"

namespace log_engine_test
{
static const char LOG_CONF_FILE_PATH[] = "./test_log_conf.properties";
static const char LOGGER_NAME[] = "my_server";

LogEngineInterface* g_log_engine;

LogEngineTest::LogEngineTest() : loader_()
{
    log_engine_ = nullptr;
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

    log_engine_ = static_cast<LogEngineInterface*>(loader_.GetModuleInterface(0));
    if (nullptr == log_engine_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    std::cout << "log engine version: " << log_engine_->GetVersion() << std::endl;

    LogEngineCtx log_engine_ctx;
    log_engine_ctx.log_conf_file_path = LOG_CONF_FILE_PATH;
    log_engine_ctx.logger_name = LOGGER_NAME;

    if (log_engine_->Initialize(&log_engine_ctx) != 0)
    {
        FAIL() << log_engine_->GetLastErrMsg();
    }

    if (log_engine_->Activate() != 0)
    {
        FAIL() << log_engine_->GetLastErrMsg();
    }

    g_log_engine = log_engine_;
}

void LogEngineTest::TearDown()
{
    SAFE_DESTROY_MODULE(log_engine_, loader_);
}

void LogEngineTest::Test001()
{
//    while (true)
//    {
    LOG_TRACE("hello");
    LOG_DEBUG("hello");
    LOG_INFO("hello");
    LOG_WARN("hello");
    LOG_ERROR("hello");
    LOG_FATAL("hello");
    LOG_ALWAYS("hello");

//        sleep(1);
//    }
}

void LogEngineTest::Test002()
{
    LOG_TRACE("hello");
    LOG_DEBUG("hello");
    LOG_INFO("hello");
    LOG_WARN("hello");
    LOG_ERROR("hello");
    LOG_FATAL("hello");
    LOG_ALWAYS("hello");

//    while (true)
//    {
//        // ... 手动修改日志配置文件，然后reload
//        log_engine_->Reload();

    LOG_TRACE("hello");
    LOG_DEBUG("hello");
    LOG_INFO("hello");
    LOG_WARN("hello");
    LOG_ERROR("hello");
    LOG_FATAL("hello");
    LOG_ALWAYS("hello");
//    }

    // 修改日志级别
    g_log_engine->SetLogLevel(log4cplus::INFO_LOG_LEVEL);

    LOG_TRACE("hello");
    LOG_DEBUG("hello");
    LOG_INFO("hello");
    LOG_WARN("hello");
    LOG_ERROR("hello");
    LOG_FATAL("hello");
    LOG_ALWAYS("hello");
}

void LogEngineTest::ConsoleLogEngineTest()
{
    ConsoleLogEngine console_log_engine;
    g_log_engine = &console_log_engine;

    LOG_TRACE("console log engine");
    LOG_DEBUG("hello");
    LOG_INFO("hello");
    LOG_WARN("hello");
    LOG_ERROR("hello");
    LOG_FATAL("hello");
    LOG_ALWAYS("console log engine");

    // 修改日志级别
    g_log_engine->SetLogLevel(log4cplus::INFO_LOG_LEVEL);

    LOG_TRACE("console log engine");
    LOG_DEBUG("hello");
    LOG_INFO("hello");
    LOG_WARN("hello");
    LOG_ERROR("hello");
    LOG_FATAL("hello");
    LOG_ALWAYS("console log engine");
}

ADD_TEST_F(LogEngineTest, Test001);
ADD_TEST_F(LogEngineTest, Test002);
ADD_TEST_F(LogEngineTest, ConsoleLogEngineTest);
}
