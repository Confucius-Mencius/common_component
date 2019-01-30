#ifndef APP_LAUNCHER_SRC_APP_LAUNCHER_H_
#define APP_LAUNCHER_SRC_APP_LAUNCHER_H_

#include "app_frame_interface.h"
#include "event_wrapper.h"
#include "file_util.h"
#include "mem_util.h"
#include "service_mgr.h"

namespace app_launcher
{
struct AppLauncherCtx
{
    int argc;
    char** argv;
    char common_component_dir[MAX_PATH_LEN];
    char log_conf_file_path[MAX_PATH_LEN];
    char logger_name[MAX_LOGGER_NAME_LEN + 1];
    char app_conf_file_path[MAX_PATH_LEN];
    char cur_working_dir[MAX_PATH_LEN];
    char app_name[MAX_PATH_LEN];

    AppLauncherCtx()
    {
        argc = 0;
        argv = NULL;
        memset(common_component_dir, 0, sizeof(common_component_dir));
        memset(log_conf_file_path, 0, sizeof(log_conf_file_path));
        memset(logger_name, 0, sizeof(logger_name));
        memset(app_conf_file_path, 0, sizeof(app_conf_file_path));
        memset(cur_working_dir, 0, sizeof(cur_working_dir));
        memset(app_name, 0, sizeof(app_name));
    }
};

class AppLauncher
{
    CREATE_FUNC(AppLauncher)

public:
    AppLauncher();
    ~AppLauncher();

    struct event_base* GetThreadEvBase() const
    {
        return thread_ev_base_;
    }

    const AppLauncherCtx* GetAppLauncherCtx() const
    {
        return app_launcher_ctx_;
    }

    AppFrameInterface* GetAppFrame() const
    {
        return app_frame_;
    }

    const char* GetLastErrMsg() const;
    void Release();
    int Initialize(const AppLauncherCtx* app_launcher_ctx);
    void Finalize();
    int Activate();
    void Freeze();

    void OnReload(bool app_conf_changed, bool log_conf_changed); // reload conf file
    void OnExitCheck();

    void Stop();

private:
    int SingleRunCheck(); // lock a unique file for only a single app instance
    int RecordPID(); // record process id
    int LoadAppFrame(const char* common_component_dir);

private:
    LastErrMsg last_err_msg_;

    const AppLauncherCtx* app_launcher_ctx_;
    ServiceMgr service_mgr_;
    struct event_base* thread_ev_base_;
    EventWrapper event_wrapper_;

    ModuleLoader app_frame_loader_;
    AppFrameInterface* app_frame_;
};
}

#endif // APP_LAUNCHER_SRC_APP_LAUNCHER_H_
