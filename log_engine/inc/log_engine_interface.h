#ifndef LOG_ENGINE_INC_LOG_ENGINE_INTERFACE_H_
#define LOG_ENGINE_INC_LOG_ENGINE_INTERFACE_H_

#include <stddef.h>
#include "module_interface.h"

#define MAX_LOGGER_NAME_LEN 31

namespace log4cplus
{
class Logger;
}

struct LogEngineCtx
{
    const char* log_conf_file_path;
    const char* logger_name;

    LogEngineCtx()
    {
        log_conf_file_path = NULL;
        logger_name = NULL;
    }
};

class LogEngineInterface : public ModuleInterface
{
public:
    virtual ~LogEngineInterface()
    {
    }

    virtual const log4cplus::Logger& GetLogger() const = 0;
    virtual int Reload() = 0;
};

#endif // LOG_ENGINE_INC_LOG_ENGINE_INTERFACE_H_
