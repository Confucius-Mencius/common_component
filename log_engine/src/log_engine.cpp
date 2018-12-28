#include "log_engine.h"
#include <log4cplus/fileappender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/loggingmacros.h>
#include "str_util.h"
#include "version.h"

// 对配置脚本进行监控，一旦发现配置脚本被更新则立刻重新加载配置。
// log4cplus::ConfigureAndWatchThread configureThread("log4cplus.properties", 5 * 1000);

namespace log_engine
{
LogEngine::LogEngine() : last_err_msg_(), log_engine_ctx_(), logger_()
{
    log_conf_file_watch_thread_ = NULL;
}

LogEngine::~LogEngine()
{
    log4cplus::Logger::shutdown(); // 放在最后关闭，使得在其它模块的Release接口中也能正常打印日志
}

const char* LogEngine::GetVersion() const
{
    return LOG_ENGINE_VERSION;
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
    if (NULL == ctx)
    {
        return -1;
    }

    log_engine_ctx_ = *((LogEngineCtx*) ctx);
    log4cplus::initialize();

    try
    {
        // 从配置文件中获取logger、appender和layout
        log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(log_engine_ctx_.log_conf_file_path));

        if (!log4cplus::Logger::exists(LOG4CPLUS_TEXT(log_engine_ctx_.logger_name)))
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "can not find logger name " << log_engine_ctx_.logger_name
                             << " in file " << log_engine_ctx_.log_conf_file_path);
            return -1;
        }

        logger_ = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(log_engine_ctx_.logger_name));

        // 对配置脚本进行监控，一旦发现配置脚本被更新则立刻重新加载配置
        log_conf_file_watch_thread_ = new log4cplus::ConfigureAndWatchThread(
            LOG4CPLUS_TEXT(log_engine_ctx_.log_conf_file_path), log_engine_ctx_.log_conf_file_check_interval * 1000);
        if (NULL == log_conf_file_watch_thread_)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "failed to create log conf file watch thread");
            return -1;
        }
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
    if (log_conf_file_watch_thread_ != NULL)
    {
        delete log_conf_file_watch_thread_;
        log_conf_file_watch_thread_ = NULL;
    }
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
}
