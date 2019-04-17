#ifndef LOG_ENGINE_INC_LOG_ENGINE_INTERFACE_H_
#define LOG_ENGINE_INC_LOG_ENGINE_INTERFACE_H_

#include <log4cplus/loglevel.h>
#include "module_interface.h"

/**
  * logger name的最大长度
  */
#define MAX_LOGGER_NAME_LEN 31

namespace log4cplus
{
class Logger;
}

struct LogEngineCtx
{
    const char* log_conf_file_path; // log配置文件
    const char* logger_name;

    LogEngineCtx()
    {
        log_conf_file_path = nullptr;
        logger_name = nullptr;
    }
};

class LogEngineInterface : public ModuleInterface
{
public:
    virtual ~LogEngineInterface()
    {
    }

    virtual const log4cplus::Logger& GetLogger() const = 0;
    virtual void SetLogLevel(int level) = 0;
    virtual int Reload() = 0;
};

// define our custom loglevel ALWAYS
const log4cplus::LogLevel ALWAYS_LOG_LEVEL = 55000;

#define LOG4CPLUS_ALWAYS(logger, logEvent) \
    if (logger.isEnabledFor(ALWAYS_LOG_LEVEL)) { \
        log4cplus::tostringstream _log4cplus_buf; \
        _log4cplus_buf << logEvent; \
        logger.forcedLog(ALWAYS_LOG_LEVEL, _log4cplus_buf.str(), LOG4CPLUS_MACRO_FILE (), __LINE__, LOG4CPLUS_MACRO_FUNCTION());}

static const log4cplus::tstring _ALWAYS_STRING(LOG4CPLUS_TEXT("ALWAYS"));

inline const log4cplus::tstring& alwaysToStringMethod(log4cplus::LogLevel ll)
{
    static log4cplus::tstring empty_str("");
    if (ll == ALWAYS_LOG_LEVEL)
    {
        return _ALWAYS_STRING;
    }

    return empty_str;
}

inline log4cplus::LogLevel alwaysFromStringMethod(const log4cplus::tstring& s)
{
    if (s == _ALWAYS_STRING)
    {
        return ALWAYS_LOG_LEVEL;
    }

    return log4cplus::NOT_SET_LOG_LEVEL;
}

#endif // LOG_ENGINE_INC_LOG_ENGINE_INTERFACE_H_
