#ifndef LOG_ENGINE_INC_CONSOLE_LOG_ENGINE_H_
#define LOG_ENGINE_INC_CONSOLE_LOG_ENGINE_H_

#include <log4cplus/consoleappender.h>
#include <log4cplus/logger.h>
#include "log_engine_interface.h"

/**
 * @brief 方便测试用例中使用的console log engine
 */
class ConsoleLogEngine : public LogEngineInterface
{
public:
    ConsoleLogEngine() : log_engine_ctx_()
    {
        // 定义Logger
        logger_ = log4cplus::Logger::getInstance("console_logger");

        // 定义一个控制台的Appender
        log4cplus::SharedAppenderPtr console_appender(new log4cplus::ConsoleAppender());

        // layout
        log4cplus::tstring pattern = LOG4CPLUS_TEXT("[%-5p %c %t %D{%Y-%m-%d %H:%M:%S %Q} %b:%L] %m%n");
        console_appender->setLayout(std::unique_ptr<log4cplus::Layout>(new log4cplus::PatternLayout(pattern)));

        // 将需要关联Logger的Appender添加到Logger上
        logger_.addAppender(console_appender);

        // 所有的日志级别都打印
        logger_.setLogLevel(log4cplus::ALL_LOG_LEVEL);
    }

    virtual ~ConsoleLogEngine()
    {
    }

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override
    {
        return NULL;
    }

    const char* GetLastErrMsg() const override
    {
        return NULL;
    }

    void Release() override
    {
    }

    int Initialize(const void* ctx) override
    {
        (void) ctx;
        return 0;
    }

    void Finalize() override
    {
    }

    int Activate() override
    {
        return 0;
    }

    void Freeze() override
    {
    }

    ///////////////////////// LogEngineInterface /////////////////////////
    const log4cplus::Logger& GetLogger() const override
    {
        return logger_;
    }

    void SetLogLevel(int level) override
    {
        logger_.setLogLevel(level);
    }

private:
    LogEngineCtx log_engine_ctx_;
    log4cplus::Logger logger_;
};

#endif // LOG_ENGINE_INC_CONSOLE_LOG_ENGINE_H_
