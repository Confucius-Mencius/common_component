#include "common_logic.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "file_util.h"
#include "log_util.h"
#include "mem_util.h"
#include "proto_msg_codec.h"
#include "work_scheduler_interface.h"

namespace work
{
enum
{
    EXIT_CHECK_TIMER_ID = 1,
};

GlobalCommonLogic::GlobalCommonLogic() : global_logic_args_(), global_logic_loader_(), scheduler_(), msg_dispatcher_()
{
    global_logic_ = nullptr;
}

GlobalCommonLogic::~GlobalCommonLogic()
{
}

const char* GlobalCommonLogic::GetVersion() const
{
    return nullptr;
}

const char* GlobalCommonLogic::GetLastErrMsg() const
{
    return nullptr;
}

void GlobalCommonLogic::Release()
{
    SAFE_RELEASE_MODULE(global_logic_, global_logic_loader_);

    delete this;
}

int GlobalCommonLogic::Initialize(const void* ctx)
{
    if (LogicInterface::Initialize(ctx) != 0)
    {
        return -1;
    }

    global_logic_args_ = *((GlobalLogicArgs*) logic_ctx_.logic_args);

    scheduler_.SetWorkScheduler(logic_ctx_.scheduler);
    scheduler_.SetRelatedThreadGroups(global_logic_args_.related_thread_groups);

    if (LoadGlobalLogic() != 0)
    {
        return -1;
    }

    // 传出去
    ((GlobalLogicArgs*) logic_ctx_.logic_args)->global_logic = global_logic_;

    return 0;
}

void GlobalCommonLogic::Finalize()
{
    SAFE_FINALIZE(global_logic_);
}

int GlobalCommonLogic::Activate()
{
    if (SAFE_ACTIVATE_FAILED(global_logic_))
    {
        return -1;
    }

    return 0;
}

void GlobalCommonLogic::Freeze()
{
    SAFE_FREEZE(global_logic_);
}

void GlobalCommonLogic::OnStop()
{
    if (global_logic_ != nullptr)
    {
        global_logic_->OnStop();
    }

    // 启动定时器，检查proto tcp logics是否都可以退出了。100毫秒检查一次
    struct timeval tv = { 0, 100000 };

    if (logic_ctx_.timer_axis->SetTimer(this, EXIT_CHECK_TIMER_ID, tv, nullptr, 0) != 0)
    {
        LOG_ERROR("failed to start exit check timer");
        return;
    }
}

void GlobalCommonLogic::OnReload()
{
    if (global_logic_ != nullptr)
    {
        global_logic_->OnReload();
    }
}

void GlobalCommonLogic::OnTask(const ConnGUID* conn_guid, ThreadInterface* source_thread, const void* data, size_t len)
{
    if (nullptr == data || 0 == len)
    {
        LOG_ERROR("invalid params");
        return;
    }

    ::proto::MsgID err_msg_id;
    ::proto::MsgHead msg_head;
    char* msg_body = nullptr;
    size_t msg_body_len = 0;

    if (logic_ctx_.msg_codec->DecodeMsg(err_msg_id, &msg_head, &msg_body, msg_body_len, (const char*) data, len) != 0)
    {
        return;
    }

    if (0 == msg_dispatcher_.DispatchMsg(conn_guid, msg_head, msg_body, msg_body_len))
    {
        if (conn_guid != nullptr)
        {
            LOG_TRACE("dispatch msg ok, " << *conn_guid << ", msg id: " << msg_head.msg_id);
        }
        else
        {
            LOG_TRACE("dispatch msg ok, msg id: " << msg_head.msg_id);
        }
    }
    else
    {
        LOG_ERROR("failed to dispatch msg, msg id: " << msg_head.msg_id);
        return;
    }
}

void GlobalCommonLogic::OnTimer(TimerID timer_id, void* data, size_t len, int times)
{
    int can_exit = 1;

    if (global_logic_ != nullptr)
    {
        can_exit &= (global_logic_->CanExit() ? 1 : 0);
    }

    if (can_exit != 0)
    {
        can_exit_ = true;
    }
}

int GlobalCommonLogic::LoadGlobalLogic()
{
    const std::string logic_so = global_logic_args_.app_frame_conf_mgr->GetGlobalLogicSo();
    if (0 == logic_so.length())
    {
        return 0;
    }

    char logic_so_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(logic_so_path, sizeof(logic_so_path),
                    logic_so.c_str(), logic_ctx_.cur_working_dir);
    LOG_ALWAYS("load logic so " << logic_so_path << " begin");

    if (global_logic_loader_.Load(logic_so_path) != 0)
    {
        LOG_ERROR("failed to load logic so, " << global_logic_loader_.GetLastErrMsg());
        return -1;
    }

    global_logic_ = static_cast<global::LogicInterface*>(global_logic_loader_.GetModuleInterface());
    if (nullptr == global_logic_)
    {
        LOG_ERROR("failed to get logic, " << global_logic_loader_.GetLastErrMsg());
        return -1;
    }

    global::LogicCtx logic_ctx;
    logic_ctx.argc = logic_ctx_.argc;
    logic_ctx.argv = logic_ctx_.argv;
    logic_ctx.common_component_dir = logic_ctx_.common_component_dir;
    logic_ctx.cur_working_dir = logic_ctx_.cur_working_dir;
    logic_ctx.app_name = logic_ctx_.app_name;
    logic_ctx.conf_center = logic_ctx_.conf_center;
    logic_ctx.timer_axis = logic_ctx_.timer_axis;
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.msg_dispatcher = &msg_dispatcher_;
    logic_ctx.global_logic = global_logic_;
    logic_ctx.thread_ev_base = logic_ctx_.thread_ev_base;

    if (global_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_ALWAYS("load logic so " << logic_so_path << " end");
    return 0;
}
}
