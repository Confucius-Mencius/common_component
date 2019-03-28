#include "log_engine.h"
#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/loggingmacros.h>
#include "str_util.h"
#include "version.h"

namespace log_engine
{
LogEngine::LogEngine() : last_err_msg_(), log_engine_ctx_(), logger_()
{
}

LogEngine::~LogEngine()
{
}

const char* LogEngine::GetVersion() const
{
    return LOG_ENGINE_LOG_ENGINE_VERSION;
}

const char* LogEngine::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

void LogEngine::Release()
{
    delete this;
}

int LogEngine::Initialize(const void* ctx)
{
    if (nullptr == ctx)
    {
        return -1;
    }

    log_engine_ctx_ = *(static_cast<const LogEngineCtx*>(ctx));

    try
    {
        // 从配置文件中获取logger、appender和layout
        log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_C_STR_TO_TSTRING(log_engine_ctx_.log_conf_file_path));

        if (!log4cplus::Logger::exists(LOG4CPLUS_C_STR_TO_TSTRING(log_engine_ctx_.logger_name)))
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "can not find logger name " << log_engine_ctx_.logger_name
                             << " in file " << log_engine_ctx_.log_conf_file_path);
            return -1;
        }

        logger_ = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(log_engine_ctx_.logger_name));
    }
    catch (...)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "log4cplus caught exception");
        return -1;
    }

    return 0;
}

void LogEngine::Finalize()
{
}

int LogEngine::Activate()
{
    return 0;
}

void LogEngine::Freeze()
{
}

const log4cplus::Logger& LogEngine::GetLogger() const
{
    return logger_;
}

void LogEngine::SetLogLevel(int level)
{
    logger_.setLogLevel(level);
}

int LogEngine::Reload()
{
    log4cplus::Logger::getRoot().closeNestedAppenders();
    log4cplus::Logger::getRoot().removeAllAppenders();

    // my named logger
    logger_.closeNestedAppenders();
    logger_.removeAllAppenders();

    log4cplus::Logger::getRoot().getHierarchy().resetConfiguration();
    logger_.getHierarchy().resetConfiguration();

    try
    {
        // 从配置文件中获取logger、appender和layout
        log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_C_STR_TO_TSTRING(log_engine_ctx_.log_conf_file_path));

        if (!log4cplus::Logger::exists(LOG4CPLUS_C_STR_TO_TSTRING(log_engine_ctx_.logger_name)))
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "can not find logger name " << log_engine_ctx_.logger_name
                             << " in file " << log_engine_ctx_.log_conf_file_path);
            return -1;
        }

        logger_ = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(log_engine_ctx_.logger_name));
    }
    catch (...)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "log4cplus caught exception");
        return -1;
    }

    return 0;
}
}
