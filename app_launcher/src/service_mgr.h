#ifndef APP_LAUNCHER_SRC_SERVICE_MGR_H_
#define APP_LAUNCHER_SRC_SERVICE_MGR_H_

// 加载对app_frame提供的服务so

#include "conf_mgr.h"
#include "module_loader.h"
#include "log_engine_interface.h"
#include "thread_center_interface.h"

namespace app_launcher
{
struct AppLauncherCtx;

class ServiceMgr
{
public:
    static void EventLogCallback(int severity, const char* msg);

public:
    ServiceMgr();
    ~ServiceMgr();

    ConfCenterInterface* GetConfCenter() const
    {
        return conf_center_;
    }

    app_launcher::ConfMgr* GetConfMgr() const
    {
        return const_cast<app_launcher::ConfMgr*>(&conf_mgr_);
    }

    ThreadCenterInterface* GetThreadCenter() const
    {
        return thread_center_;
    }

    void Reload(bool& app_conf_changed);

    const char* GetLastErrMsg() const;
    void Release();
    int Initialize(const AppLauncherCtx* app_launcher_ctx);
    void Finalize();
    int Activate();
    void Freeze();

private:
    int LoadConfCenter();
    int LoadLogEngine();
    int LoadThreadCenter();

private:
    LastErrMsg last_err_msg_;
    const AppLauncherCtx* app_launcher_ctx_;

    ModuleLoader conf_center_loader_;
    ConfCenterInterface* conf_center_;

    ModuleLoader log_engine_loader_;
    LogEngineInterface* log_engine_;

    app_launcher::ConfMgr conf_mgr_;

    ModuleLoader thread_center_loader_;
    ThreadCenterInterface* thread_center_;
};
}

#endif // APP_LAUNCHER_SRC_SERVICE_MGR_H_
