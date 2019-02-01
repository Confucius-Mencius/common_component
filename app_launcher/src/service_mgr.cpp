#include "service_mgr.h"
#include "app_launcher.h"
#include "num_util.h"
#include "str_util.h"

LogEngineInterface* g_log_engine = NULL;

namespace app_launcher
{
void ServiceMgr::EventLogCallback(int severity, const char* msg)
{
    switch (severity)
    {
        case _EVENT_LOG_DEBUG:
        {
            LOG_DEBUG(msg);
        }
        break;

        case _EVENT_LOG_MSG:
        {
            LOG_INFO(msg);
        }
        break;

        case _EVENT_LOG_WARN:
        {
            LOG_WARN(msg);
        }
        break;

        case _EVENT_LOG_ERR:
        {
            LOG_ERROR(msg);
        }
        break;

        default:
        {

        }
        break;
    }
}

ServiceMgr::ServiceMgr() : last_err_msg_(), log_engine_loader_(), conf_center_loader_(),
    thread_center_loader_()
{
    app_launcher_ = NULL;
    log_engine_ = NULL;
    conf_center_ = NULL;
    thread_center_ = NULL;
}

ServiceMgr::~ServiceMgr()
{
}

void ServiceMgr::Reload(bool app_conf_changed, bool log_conf_changed)
{
    if (log_conf_changed && log_engine_->Reload() != 0)
    {
        LOG_ERROR("failed to reload log conf");
    }

    if (app_conf_changed && conf_center_->Reload() != 0)
    {
        LOG_ERROR("failed to reload app conf");
    }
}

const char* ServiceMgr::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

void ServiceMgr::Release()
{
    SAFE_RELEASE_MODULE(thread_center_, thread_center_loader_);
    SAFE_RELEASE_MODULE(conf_center_, conf_center_loader_);
    SAFE_RELEASE_MODULE(log_engine_, log_engine_loader_);
    g_log_engine = NULL;
}

int ServiceMgr::Initialize()
{
    if (LoadLogEngine() != 0)
    {
        return -1;
    }

    if (LoadConfCenter() != 0)
    {
        return -1;
    }

    if (LoadThreadCenter() != 0)
    {
        return -1;
    }

    return 0;
}

void ServiceMgr::Finalize()
{
    SAFE_FINALIZE(thread_center_);
    SAFE_FINALIZE(conf_center_);
    SAFE_FINALIZE(log_engine_);
}

int ServiceMgr::Activate()
{
    if (SAFE_ACTIVATE_FAILED(log_engine_))
    {
        LOG_ERROR(log_engine_->GetLastErrMsg());
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(conf_center_))
    {
        LOG_ERROR(conf_center_->GetLastErrMsg());
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(thread_center_))
    {
        LOG_ERROR("failed to activate thread center");
        return -1;
    }

    return 0;
}

void ServiceMgr::Freeze()
{
    SAFE_FREEZE(thread_center_);
    SAFE_FREEZE(conf_center_);
    SAFE_FREEZE(log_engine_);
}

int ServiceMgr::LoadLogEngine()
{
    char log_engine_so_path[MAX_PATH_LEN] = "";
    StrPrintf(log_engine_so_path, sizeof(log_engine_so_path), "%s/liblog_engine.so",
              app_launcher_->GetAppLauncherCtx()->common_component_dir);

    if (log_engine_loader_.Load(log_engine_so_path) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, log_engine_loader_.GetLastErrMsg());
        return -1;
    }

    log_engine_ = static_cast<LogEngineInterface*>(log_engine_loader_.GetModuleInterface(0));
    if (NULL == log_engine_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, log_engine_loader_.GetLastErrMsg());
        return -1;
    }

    LogEngineCtx log_engine_ctx;
    log_engine_ctx.log_conf_file_path = app_launcher_->GetAppLauncherCtx()->log_conf_file_path;
    log_engine_ctx.logger_name = app_launcher_->GetAppLauncherCtx()->logger_name;

    if (log_engine_->Initialize(&log_engine_ctx) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, log_engine_->GetLastErrMsg());
        return -1;
    }

    g_log_engine = log_engine_; // 此后运行的代码就可以使用LOG_XXX宏记录日志了
    event_set_log_callback(ServiceMgr::EventLogCallback);

    return 0;
}

int ServiceMgr::LoadConfCenter()
{
    char conf_center_so_path[MAX_PATH_LEN] = "";
    StrPrintf(conf_center_so_path, sizeof(conf_center_so_path), "%s/libconf_center.so",
              app_launcher_->GetAppLauncherCtx()->common_component_dir);

    if (conf_center_loader_.Load(conf_center_so_path) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, conf_center_loader_.GetLastErrMsg());
        return -1;
    }

    conf_center_ = static_cast<ConfCenterInterface*>(conf_center_loader_.GetModuleInterface(0));
    if (NULL == conf_center_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, conf_center_loader_.GetLastErrMsg());
        return -1;
    }

    ConfCenterCtx conf_center_ctx;
    conf_center_ctx.app_conf_file_path = app_launcher_->GetAppLauncherCtx()->app_conf_file_path;

    if (conf_center_->Initialize(&conf_center_ctx) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, conf_center_->GetLastErrMsg());
        return -1;
    }

    return 0;
}

int ServiceMgr::LoadThreadCenter()
{
    char thread_center_so_path[MAX_PATH_LEN] = "";
    StrPrintf(thread_center_so_path, sizeof(thread_center_so_path), "%s/libthread_center.so",
              app_launcher_->GetAppLauncherCtx()->common_component_dir);

    if (thread_center_loader_.Load(thread_center_so_path) != 0)
    {
        LOG_ERROR(thread_center_loader_.GetLastErrMsg());
        return -1;
    }

    thread_center_ = static_cast<ThreadCenterInterface*>(thread_center_loader_.GetModuleInterface());
    if (NULL == thread_center_)
    {
        LOG_ERROR(thread_center_loader_.GetLastErrMsg());
        return -1;
    }

    if (thread_center_->Initialize(NULL) != 0)
    {
        return -1;
    }

    return 0;
}
}
