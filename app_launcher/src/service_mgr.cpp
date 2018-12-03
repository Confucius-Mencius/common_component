#include "service_mgr.h"
#include "app_launcher.h"
#include "num_util.h"

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
    }
}

ServiceMgr::ServiceMgr() : last_err_msg_(), log_engine_loader_(), conf_center_loader_(), conf_mgr_(),
                           msg_codec_center_loader_(), thread_center_loader_()
{
    app_launcher_ctx_ = NULL;
    log_engine_ = NULL;
    conf_center_ = NULL;
    msg_codec_center_ = NULL;
    thread_center_ = NULL;
}

ServiceMgr::~ServiceMgr()
{
}

void ServiceMgr::Reload(bool& app_conf_changed)
{
    if (log_engine_->Reload() != 0)
    {
        LOG_ERROR("failed to reload log conf");
    }
//    else
//    {
//        LOG_INFO("reload log conf ok");
//    }

    if (conf_center_->Reload(app_conf_changed) != 0)
    {
        LOG_ERROR("failed to reload app conf");
    }
//    else
//    {
//        LOG_INFO("reload app conf ok");
//    }

    if (app_conf_changed)
    {
        if (conf_mgr_.Reload() != 0)
        {
            LOG_ERROR("conf mgr reload failed");
        }
    }
}

const char* ServiceMgr::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

void ServiceMgr::Release()
{
    ////////////////////////////////////////////////////////////////////////////////
    SAFE_RELEASE_MODULE(thread_center_, thread_center_loader_);
    SAFE_RELEASE_MODULE(msg_codec_center_, msg_codec_center_loader_);
    SAFE_RELEASE_MODULE(conf_center_, conf_center_loader_);
    SAFE_RELEASE_MODULE(log_engine_, log_engine_loader_);
    g_log_engine = NULL;
}

int ServiceMgr::Initialize(const AppLauncherCtx* app_launcher_ctx)
{
    if (NULL == app_launcher_ctx)
    {
        return -1;
    }

    app_launcher_ctx_ = app_launcher_ctx;

    if (LoadLogEngine() != 0)
    {
        return -1;
    }

    if (LoadConfCenter() != 0)
    {
        return -1;
    }

    if (conf_mgr_.Initialize(conf_center_) != 0)
    {
        return -1;
    }

    if (LoadMsgCodecCenter() != 0)
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
    SAFE_FINALIZE(msg_codec_center_);
    conf_mgr_.Finalize();
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

    if (SAFE_ACTIVATE_FAILED(msg_codec_center_))
    {
        LOG_ERROR("failed to activate msg codec center");
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
    SAFE_FREEZE(msg_codec_center_);
    SAFE_FREEZE(conf_center_);
    SAFE_FREEZE(log_engine_);
}

int ServiceMgr::LoadLogEngine()
{
    char LOG_ENGINE_SO_PATH[MAX_PATH_LEN + 1] = "";
    StrPrintf(LOG_ENGINE_SO_PATH, sizeof(LOG_ENGINE_SO_PATH), "%s/liblog_engine.so",
              app_launcher_ctx_->common_component_dir);

    if (log_engine_loader_.Load(LOG_ENGINE_SO_PATH) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, log_engine_loader_.GetLastErrMsg());
        return -1;
    }

    log_engine_ = (LogEngineInterface*) log_engine_loader_.GetModuleInterface(0);
    if (NULL == log_engine_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, log_engine_loader_.GetLastErrMsg());
        return -1;
    }

    LogEngineCtx log_engine_ctx;
    log_engine_ctx.log_conf_file_path = app_launcher_ctx_->log_conf_file_path;
    log_engine_ctx.logger_name = app_launcher_ctx_->logger_name;

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
    char CONF_CENTER_SO_PATH[MAX_PATH_LEN + 1] = "";
    StrPrintf(CONF_CENTER_SO_PATH, sizeof(CONF_CENTER_SO_PATH), "%s/libconf_center.so",
              app_launcher_ctx_->common_component_dir);

    if (conf_center_loader_.Load(CONF_CENTER_SO_PATH) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, conf_center_loader_.GetLastErrMsg());
        return -1;
    }

    conf_center_ = (ConfCenterInterface*) conf_center_loader_.GetModuleInterface(0);
    if (NULL == conf_center_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, conf_center_loader_.GetLastErrMsg());
        return -1;
    }

    ConfCenterCtx conf_center_ctx;
    conf_center_ctx.app_conf_file_path = app_launcher_ctx_->app_conf_file_path;

    if (conf_center_->Initialize(&conf_center_ctx) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, conf_center_->GetLastErrMsg());
        return -1;
    }

    return 0;
}

int ServiceMgr::LoadMsgCodecCenter()
{
    char MSG_CODEC_CENTER_SO_PATH[MAX_PATH_LEN + 1] = "";
    StrPrintf(MSG_CODEC_CENTER_SO_PATH, sizeof(MSG_CODEC_CENTER_SO_PATH), "%s/libmsg_codec_center.so",
              app_launcher_ctx_->common_component_dir);

    if (msg_codec_center_loader_.Load(MSG_CODEC_CENTER_SO_PATH) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, msg_codec_center_loader_.GetLastErrMsg());
        return -1;
    }

    msg_codec_center_ = (MsgCodecCenterInterface*) msg_codec_center_loader_.GetModuleInterface(0);
    if (NULL == msg_codec_center_)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, msg_codec_center_loader_.GetLastErrMsg());
        return -1;
    }

    if (msg_codec_center_->Initialize(NULL) != 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, msg_codec_center_->GetLastErrMsg());
        return -1;
    }

    return 0;
}

int ServiceMgr::LoadThreadCenter()
{
    char THREAD_CENTER_SO_PATH[MAX_PATH_LEN + 1] = "";
    StrPrintf(THREAD_CENTER_SO_PATH, sizeof(THREAD_CENTER_SO_PATH), "%s/libthread_center.so",
              app_launcher_ctx_->common_component_dir);

    if (thread_center_loader_.Load(THREAD_CENTER_SO_PATH) != 0)
    {
        LOG_ERROR(thread_center_loader_.GetLastErrMsg());
        return -1;
    }

    thread_center_ = (ThreadCenterInterface*) thread_center_loader_.GetModuleInterface();
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
