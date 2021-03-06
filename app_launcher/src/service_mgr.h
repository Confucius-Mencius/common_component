#ifndef APP_LAUNCHER_SRC_SERVICE_MGR_H_
#define APP_LAUNCHER_SRC_SERVICE_MGR_H_

// 加载对app_frame提供的服务so

#include "module_loader.h"
#include "conf_center_interface.h"
#include "log_engine_interface.h"
#include "thread_center_interface.h"

namespace app_launcher
{
class AppLauncher;

class ServiceMgr
{
public:
    static void EventLogCallback(int severity, const char* msg);

public:
    ServiceMgr();
    ~ServiceMgr();

    void SetAppLauncher(AppLauncher* app_launcher)
    {
        app_launcher_ = app_launcher;
    }

    ConfCenterInterface* GetConfCenter() const
    {
        return conf_center_;
    }

    ThreadCenterInterface* GetThreadCenter() const
    {
        return thread_center_;
    }

    void Reload(bool app_conf_changed, bool log_conf_changed);

    const char* GetLastErrMsg() const;
    void Release();
    int Initialize();
    void Finalize();
    int Activate();
    void Freeze();

private:
    int LoadLogEngine();
    int LoadConfCenter();
    int LoadThreadCenter();

private:
    LastErrMsg last_err_msg_;
    AppLauncher* app_launcher_;

    ModuleLoader log_engine_loader_;
    LogEngineInterface* log_engine_;

    ModuleLoader conf_center_loader_;
    ConfCenterInterface* conf_center_;

    ModuleLoader thread_center_loader_;
    ThreadCenterInterface* thread_center_;
};
}

#endif // APP_LAUNCHER_SRC_SERVICE_MGR_H_
