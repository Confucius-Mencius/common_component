#include "app_launcher.h"
#include "res_limits_util.h"
#include "str_util.h"
#include "util.h"
#include "version.h"

namespace app_launcher
{
void AppLauncher::OnConfCheckTimer(evutil_socket_t fd, short event, void* arg)
{
    (void) fd;
    (void) event;

    AppLauncher* app_launcher = static_cast<AppLauncher*>(arg);
    app_launcher->OnReload();
}

void AppLauncher::OnExitCheckTimer(evutil_socket_t fd, short event, void* arg)
{
    (void) fd;
    (void) event;

    LOG_TRACE("in exit check timer");
    AppLauncher* app_launcher = static_cast<AppLauncher*>(arg);

    if (!app_launcher->app_frame_->CanExit())
    {
        return;
    }

    app_launcher->app_frame_->NotifyExit();
    event_base_loopbreak(app_launcher->thread_ev_base_);
}

AppLauncher::AppLauncher() : last_err_msg_(), service_mgr_(), signal_wrapper_(), app_frame_loader_()
{
    app_launcher_ctx_ = NULL;
    thread_ev_base_ = NULL;
    app_frame_ = NULL;
    conf_check_timer_event_ = NULL;
    stopping_ = false;
    exit_check_timer_event_ = NULL;
}

AppLauncher::~AppLauncher()
{
}

const char* AppLauncher::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

void AppLauncher::Release()
{
    SAFE_RELEASE_MODULE(app_frame_, app_frame_loader_);

    if (g_log_engine != NULL)
    {
        LOG_ALWAYS(app_launcher_ctx_->app_name << " exit ok");
    }

    service_mgr_.Release();
    delete this;
}

int AppLauncher::Initialize(const AppLauncherCtx* app_launcher_ctx)
{
    if (NULL == app_launcher_ctx)
    {
        return -1;
    }

    app_launcher_ctx_ = app_launcher_ctx;

    if (SingleRunCheck() != 0)
    {
        return -1;
    }

    if (RecordPID() != 0)
    {
        return -1;
    }

    ////////////////////////////////////////////////////////////////////////////////
    if (service_mgr_.Initialize(app_launcher_ctx_) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, service_mgr_.GetLastErrMsg());
        return -1;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // !!!此后就可以用LOG_XXX打印日志了!!!
    ////////////////////////////////////////////////////////////////////////////////

    LOG_ALWAYS("app launcher version: " << APP_LAUNCHER_VERSION);
    LOG_DEBUG("argc: " << app_launcher_ctx_->argc);

    for (int i = 0; i < app_launcher_ctx_->argc; ++i)
    {
        LOG_DEBUG("argv " << i << ": " << app_launcher_ctx_->argv[i]);
    }

    LOG_ALWAYS("common component dir: " << app_launcher_ctx_->common_component_dir);
    LOG_ALWAYS("log conf file path: " << app_launcher_ctx_->log_conf_file_path);
    LOG_ALWAYS("logger name: " << app_launcher_ctx_->logger_name);
    LOG_ALWAYS("app conf file path: " << app_launcher_ctx_->app_conf_file_path);
    LOG_ALWAYS("cur working dir: " << app_launcher_ctx_->cur_working_dir);
    LOG_ALWAYS("app name: " << app_launcher_ctx_->app_name);

    ////////////////////////////////////////////////////////////////////////////////
    PrintAllResLimits();

    ////////////////////////////////////////////////////////////////////////////////
    // event_config方式
//    struct event_config* cfg = event_config_new();
//    if (NULL == cfg)
//    {
//        const int err = errno;
//        LOG_ERROR("failed to create libevent config, errno: " << err << ", err msg: " << strerror(err));
//        return -1;
//    }

//    if (event_config_require_features(cfg,  EV_FEATURE_ET | EV_FEATURE_O1 | EV_FEATURE_FDS | EV_FEATURE_EARLY_CLOSE) != 0)
//    {
//        LOG_ERROR("failed to set libevent config");
//        event_config_free(cfg);
//        return -1;
//    }

//    thread_ev_base_ = event_base_new_with_config(cfg);
    thread_ev_base_ = event_base_new();
    if (NULL == thread_ev_base_)
    {
        const int err = errno;
        LOG_ERROR("failed to create main thread event base, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    const int features = event_base_get_features(thread_ev_base_);
    if (features & EV_FEATURE_ET)
    {
        LOG_ALWAYS("libevent feature EV_FEATURE_ET supported");
    }

    if (features & EV_FEATURE_O1)
    {
        LOG_ALWAYS("libevent feature EV_FEATURE_O1 supported");
    }

    if (features & EV_FEATURE_FDS)
    {
        LOG_ALWAYS("libevent feature EV_FEATURE_FDS supported"); // linux下不支持
    }

    if (features & EV_FEATURE_EARLY_CLOSE)
    {
        LOG_ALWAYS("libevent feature EV_FEATURE_EARLY_CLOSE supported");
    }

    LOG_ALWAYS("libevent version: " << event_get_version()
               << ", libevent method: " << event_base_get_method(thread_ev_base_));

    ////////////////////////////////////////////////////////////////////////////////
    if (signal_wrapper_.Initialize(this) != 0)
    {
        LOG_ERROR(signal_wrapper_.GetLastErrMsg());
        return -1;
    }

    ////////////////////////////////////////////////////////////////////////////////
    if (LoadAppFrame(app_launcher_ctx_->common_component_dir) != 0)
    {
        return -1;
    }

    return 0;
}

void AppLauncher::Finalize()
{
    SAFE_FINALIZE(app_frame_);
    signal_wrapper_.Finalize();
    service_mgr_.Finalize();

    if (exit_check_timer_event_ != NULL)
    {
        event_free(exit_check_timer_event_);
        exit_check_timer_event_ = NULL;
    }

    if (conf_check_timer_event_ != NULL)
    {
        event_free(conf_check_timer_event_);
        conf_check_timer_event_ = NULL;
    }

    if (thread_ev_base_ != NULL)
    {
        event_base_free(thread_ev_base_);
        thread_ev_base_ = NULL;
    }
}

int AppLauncher::Activate()
{
    if (service_mgr_.Activate() != 0)
    {
        LOG_ERROR("failed to activate service mgr");
        return -1;
    }

    ////////////////////////////////////////////////////////////////////////////////
    const int conf_check_interval = 60;
    {
        conf_check_timer_event_ = event_new(thread_ev_base_, -1, EV_PERSIST, AppLauncher::OnConfCheckTimer, this);
        if (NULL == conf_check_timer_event_)
        {
            const int err = errno;
            LOG_ERROR("failed to create conf check timer event, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }

        const struct timeval tv = {conf_check_interval, 0};

        if (event_add(conf_check_timer_event_, &tv) != 0)
        {
            const int err = errno;
            LOG_ERROR("failed to add conf check timer event, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }

        LOG_ALWAYS("add conf check timer ok, interval: " << conf_check_interval << "s"); // 单位：秒
    }

    if (app_frame_->Activate() != 0)
    {
        LOG_ERROR("failed to activate app frame");
        return -1;
    }

    LOG_ALWAYS(app_launcher_ctx_->app_name << " is running now");
    event_base_dispatch(thread_ev_base_);

    return 0;
}

void AppLauncher::Freeze()
{
    SAFE_FREEZE(app_frame_);
    service_mgr_.Freeze();

    if (exit_check_timer_event_ != NULL)
    {
        event_del(exit_check_timer_event_);
    }

    if (conf_check_timer_event_ != NULL)
    {
        event_del(conf_check_timer_event_);
    }
}

void AppLauncher::OnStop()
{
    if (stopping_ || NULL == thread_ev_base_)
    {
        return;
    }

    stopping_ = true;
    app_frame_->NotifyStop();

    exit_check_timer_event_ = event_new(thread_ev_base_, -1, EV_PERSIST, AppLauncher::OnExitCheckTimer, this);
    if (NULL == exit_check_timer_event_)
    {
        const int err = errno;
        LOG_ERROR("failed to create exit check timer event, errno: " << err << ", err msg: " << strerror(err));
        return;
    }

    // 起一个退出检测定时器，200毫秒检查一次是否可以退出
    const struct timeval tv = {0, 200000};

    if (event_add(exit_check_timer_event_, &tv) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to add exit check timer event, errno: " << err << ", err msg: " << strerror(err));
        event_del(exit_check_timer_event_);
        exit_check_timer_event_ = NULL;
        return;
    }
}

void AppLauncher::OnReload()
{
    // 信号或定时器均可触发reload
    bool app_conf_changed = false;
    service_mgr_.Reload(app_conf_changed);
    app_frame_->NotifyReload(app_conf_changed);
}

int AppLauncher::SingleRunCheck()
{
    char lock_file_path[MAX_PATH_LEN] = "";

    if (StrPrintf(lock_file_path, sizeof(lock_file_path), "%s/.%s.lock", app_launcher_ctx_->cur_working_dir,
                  app_launcher_ctx_->app_name) <= 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to build app lock file path");
        return -1;
    }

    if (app_launcher::SingleRunCheck(lock_file_path) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to lock app file " << lock_file_path);
        return -1;
    }

    return 0;
}

int AppLauncher::RecordPID()
{
    char pid_file_path[MAX_PATH_LEN] = "";

    if (StrPrintf(pid_file_path, sizeof(pid_file_path), "%s/%s.pid", app_launcher_ctx_->cur_working_dir,
                  app_launcher_ctx_->app_name) <= 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to build pid file path");
        return -1;
    }

    if (app_launcher::RecordPID(pid_file_path) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to record pid to file " << pid_file_path);
        return -1;
    }

    return 0;
}

int AppLauncher::LoadAppFrame(const char* common_component_dir)
{
    char APP_FRAME_SO_PATH[MAX_PATH_LEN] = "";
    StrPrintf(APP_FRAME_SO_PATH, sizeof(APP_FRAME_SO_PATH), "%s/libapp_frame.so", common_component_dir);

    if (app_frame_loader_.Load(APP_FRAME_SO_PATH) != 0)
    {
        LOG_ERROR(app_frame_loader_.GetLastErrMsg());
        return -1;
    }

    app_frame_ = static_cast<AppFrameInterface*>(app_frame_loader_.GetModuleInterface());
    if (NULL == app_frame_)
    {
        LOG_ERROR(app_frame_loader_.GetLastErrMsg());
        return -1;
    }

    AppFrameCtx app_frame_ctx;
    app_frame_ctx.argc = app_launcher_ctx_->argc;
    app_frame_ctx.argv = app_launcher_ctx_->argv;
    app_frame_ctx.common_component_dir = app_launcher_ctx_->common_component_dir;
    app_frame_ctx.cur_working_dir = app_launcher_ctx_->cur_working_dir;
    app_frame_ctx.app_name = app_launcher_ctx_->app_name;
    app_frame_ctx.conf_center = service_mgr_.GetConfCenter();
    app_frame_ctx.thread_center = service_mgr_.GetThreadCenter();

    if (app_frame_->Initialize(&app_frame_ctx) != 0)
    {
        return -1;
    }

    return 0;
}
}
