#include "log_engine.h"
#include <log4cplus/configurator.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/hierarchy.h>
#include <log4cplus/loggingmacros.h>
#include "str_util.h"
#include "version.h"

// 对配置脚本进行监控，一旦发现配置脚本被更新则立刻重新加载配置。
// log4cplus::ConfigureAndWatchThread configureThread("log4cplus.properties", 5 * 1000);

namespace log_engine
{
LogEngine::LogEngine() : last_err_msg_(), log_engine_ctx_(), logger_(), log_conf_file_stat_()
{
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

    return LoadLogConf(log_engine_ctx_.log_conf_file_path, log_engine_ctx_.logger_name);
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
    FileStat file_stat;

    if (GetFileStat(file_stat, log_engine_ctx_.log_conf_file_path) != 0)
    {
        return -1;
    }

    if (file_stat.Equals(log_conf_file_stat_))
    {
        return 0; // 文件没变化
    }

    log_conf_file_stat_ = file_stat;

    try
    {
        // 仅支持修改日志级别
        log4cplus::PropertyConfigurator config(LOG4CPLUS_TEXT(log_engine_ctx_.log_conf_file_path));
        config.configure();

        char log_level_conf_key[64] = "";
        int n = StrPrintf(log_level_conf_key, sizeof(log_level_conf_key), "logger.%s", log_engine_ctx_.logger_name);
        if (n <= 0)
        {
            return -1;
        }

        const log4cplus::tstring& logger_level_conf = config.getProperties().getProperty(log_level_conf_key);

        if (logger_level_conf.find("TRACE") != log4cplus::tstring::npos)
        {
            logger_.setLogLevel(log4cplus::TRACE_LOG_LEVEL);
        }
        else if (logger_level_conf.find("DEBUG") != log4cplus::tstring::npos)
        {
            logger_.setLogLevel(log4cplus::DEBUG_LOG_LEVEL);
        }
        else if (logger_level_conf.find("WARN") != log4cplus::tstring::npos)
        {
            logger_.setLogLevel(log4cplus::WARN_LOG_LEVEL);
        }
        else if (logger_level_conf.find("ERROR") != log4cplus::tstring::npos)
        {
            logger_.setLogLevel(log4cplus::ERROR_LOG_LEVEL);
        }
        else if (logger_level_conf.find("FATAL") != log4cplus::tstring::npos)
        {
            logger_.setLogLevel(log4cplus::FATAL_LOG_LEVEL);
        }
    }
    catch (...)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "log4cplus caught exception");
        return -1;
    }

    return 0;
}

int LogEngine::LoadLogConf(const char* log_conf_file_path, const char* logger_name)
{
    if (!FileExist(log_conf_file_path))
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "log conf file " << log_conf_file_path << " not exist");
        return -1;
    }

    try
    {
        // 从配置文件中获取logger、appender和layout
        log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(log_conf_file_path));

        if (!log4cplus::Logger::exists(LOG4CPLUS_TEXT(logger_name)))
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "can not find logger name " << logger_name
                             << " in file " << log_conf_file_path);
            return -1;
        }

        logger_ = log4cplus::Logger::getInstance(LOG4CPLUS_TEXT(logger_name));
    }
    catch (...)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "log4cplus caught exception");
        return -1;
    }

    if (GetFileStat(log_conf_file_stat_, log_conf_file_path) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to get stat of file " << log_conf_file_path);
        return -1;
    }

    return 0;
}
}
