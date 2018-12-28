#ifndef LOG_ENGINE_SRC_LOG_ENGINE_H_
#define LOG_ENGINE_SRC_LOG_ENGINE_H_

#include <log4cplus/configurator.h>
#include <log4cplus/logger.h>
#include "last_err_msg.h"
#include "log_engine_interface.h"

namespace log_engine
{
class LogEngine : public LogEngineInterface
{
public:
    LogEngine();
    virtual ~LogEngine();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// LogEngineInterface /////////////////////////
    const log4cplus::Logger& GetLogger() const override;
    void SetLogLevel(int level) override;

private:
    int LoadLogConf(const char* log_conf_file_path, const char* logger_name);

private:
    LastErrMsg last_err_msg_;
    LogEngineCtx log_engine_ctx_;
    log4cplus::ConfigureAndWatchThread* log_conf_file_watch_thread_;
    log4cplus::Logger logger_;
};
}

#endif // LOG_ENGINE_SRC_LOG_ENGINE_H_
