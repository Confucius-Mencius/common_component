#include "common_logic.h"
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

BurdenLogic::BurdenLogic() : burden_logic_args_(), burden_common_logic_loader_(),
    burden_logic_item_vec_(), scheduler_(), msg_dispatcher_()
{
    burden_common_logic_ = nullptr;
}

BurdenLogic::~BurdenLogic()
{
}

const char* BurdenLogic::GetVersion() const
{
    return nullptr;
}

const char* BurdenLogic::GetLastErrMsg() const
{
    return nullptr;
}

void BurdenLogic::Release()
{
    for (BurdenLogicItemVec::iterator it = burden_logic_item_vec_.begin(); it != burden_logic_item_vec_.end(); ++it)
    {
        SAFE_RELEASE_MODULE(it->logic, it->logic_loader);
    }

    burden_logic_item_vec_.clear();
    SAFE_RELEASE_MODULE(burden_common_logic_, burden_common_logic_loader_);

    delete this;
}

int BurdenLogic::Initialize(const void* ctx)
{
    if (LogicInterface::Initialize(ctx) != 0)
    {
        return -1;
    }

    burden_logic_args_ = *(static_cast<const BurdenLogicArgs*>(logic_ctx_.logic_args));

    scheduler_.SetWorkScheduler(logic_ctx_.scheduler);

    if (LoadBurdenCommonLogic() != 0)
    {
        return -1;
    }

    if (LoadBurdenLogicGroup() != 0)
    {
        return -1;
    }

    return 0;
}

void BurdenLogic::Finalize()
{
    for (BurdenLogicItemVec::iterator it = burden_logic_item_vec_.begin(); it != burden_logic_item_vec_.end(); ++it)
    {
        SAFE_FINALIZE(it->logic);
    }

    SAFE_FINALIZE(burden_common_logic_);
}

int BurdenLogic::Activate()
{
    if (SAFE_ACTIVATE_FAILED(burden_common_logic_))
    {
        return -1;
    }

    for (BurdenLogicItemVec::iterator it = burden_logic_item_vec_.begin(); it != burden_logic_item_vec_.end(); ++it)
    {
        if (SAFE_ACTIVATE_FAILED(it->logic))
        {
            return -1;
        }
    }

    return 0;
}

void BurdenLogic::Freeze()
{
    for (BurdenLogicItemVec::iterator it = burden_logic_item_vec_.begin(); it != burden_logic_item_vec_.end(); ++it)
    {
        SAFE_FREEZE(it->logic);
    }

    SAFE_FREEZE(burden_common_logic_);
}

void BurdenLogic::OnStop()
{
    if (burden_common_logic_ != nullptr)
    {
        burden_common_logic_->OnStop();
    }

    for (BurdenLogicItemVec::iterator it = burden_logic_item_vec_.begin(); it != burden_logic_item_vec_.end(); ++it)
    {
        it->logic->OnStop();
    }

    // 启动定时器，检查burden logics是否都可以退出了。100毫秒检查一次
    struct timeval tv = { 0, 100000 };

    if (logic_ctx_.timer_axis->SetTimer(this, EXIT_CHECK_TIMER_ID, tv, nullptr, 0) != 0)
    {
        LOG_ERROR("failed to start exit check timer");
        return;
    }
}

void BurdenLogic::OnReload()
{
    if (burden_common_logic_ != nullptr)
    {
        burden_common_logic_->OnReload();
    }

    for (BurdenLogicItemVec::iterator it = burden_logic_item_vec_.begin(); it != burden_logic_item_vec_.end(); ++it)
    {
        it->logic->OnReload();
    }
}

void BurdenLogic::OnTask(const ConnGUID* conn_guid, ThreadInterface* source_thread, const void* data, size_t len)
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

void BurdenLogic::OnTimer(TimerID timer_id, void* data, size_t len, int times)
{
    int can_exit = 1;

    if (burden_common_logic_ != nullptr)
    {
        can_exit &= (burden_common_logic_->CanExit() ? 1 : 0);
    }

    for (BurdenLogicItemVec::const_iterator it = burden_logic_item_vec_.cbegin(); it != burden_logic_item_vec_.cend(); ++it)
    {
        can_exit &= (it->logic->CanExit() ? 1 : 0);
    }

    if (can_exit != 0)
    {
        can_exit_ = true;
    }
}


int BurdenLogic::LoadBurdenCommonLogic()
{
    const std::string common_logic_so = burden_logic_args_.app_frame_conf_mgr->GetBurdenCommonLogicSo();
    if (0 == common_logic_so.length())
    {
        return 0;
    }

    char common_logic_so_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(common_logic_so_path, sizeof(common_logic_so_path),
                    common_logic_so.c_str(), logic_ctx_.cur_working_dir);
    LOG_ALWAYS("load common logic so " << common_logic_so_path << " begin");

    if (burden_common_logic_loader_.Load(common_logic_so_path) != 0)
    {
        LOG_ERROR("failed to load common logic so, " << burden_common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    burden_common_logic_ = static_cast<burden::CommonLogicInterface*>(burden_common_logic_loader_.GetModuleInterface());
    if (nullptr == burden_common_logic_)
    {
        LOG_ERROR("failed to get common logic, " << burden_common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    burden::LogicCtx logic_ctx;
    logic_ctx.argc = logic_ctx_.argc;
    logic_ctx.argv = logic_ctx_.argv;
    logic_ctx.common_component_dir = logic_ctx_.common_component_dir;
    logic_ctx.cur_working_dir = logic_ctx_.cur_working_dir;
    logic_ctx.app_name = logic_ctx_.app_name;
    logic_ctx.conf_center = logic_ctx_.conf_center;
    logic_ctx.timer_axis = logic_ctx_.timer_axis;
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.msg_dispatcher = &msg_dispatcher_;
    logic_ctx.common_logic = burden_common_logic_;
    logic_ctx.thread_ev_base = logic_ctx_.thread_ev_base;
    logic_ctx.thread_idx = logic_ctx_.thread_idx;

    if (burden_common_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_ALWAYS("load common logic so " << common_logic_so_path << " end");
    return 0;
}

int BurdenLogic::LoadBurdenLogicGroup()
{
    // logic so group
    if (0 == burden_logic_args_.app_frame_conf_mgr->GetBurdenLogicSoGroup().size())
    {
        return 0;
    }

    const StrGroup logic_so_group = burden_logic_args_.app_frame_conf_mgr->GetBurdenLogicSoGroup();

    for (StrGroup::const_iterator it = logic_so_group.cbegin(); it != logic_so_group.cend(); ++it)
    {
        char logic_so_path[MAX_PATH_LEN] = "";
        GetAbsolutePath(logic_so_path, sizeof(logic_so_path), (*it).c_str(), logic_ctx_.cur_working_dir);

        BurdenLogicItem logic_item;
        logic_item.logic_so_path = logic_so_path;
        logic_item.logic = nullptr;

        burden_logic_item_vec_.push_back(logic_item);
    }

    for (BurdenLogicItemVec::iterator it = burden_logic_item_vec_.begin(); it != burden_logic_item_vec_.end(); ++it)
    {
        BurdenLogicItem& logic_item = *it;
        LOG_ALWAYS("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = static_cast<burden::LogicInterface*>(logic_item.logic_loader.GetModuleInterface());
        if (nullptr == logic_item.logic)
        {
            LOG_ERROR("failed to get logic, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        burden::LogicCtx logic_ctx;
        logic_ctx.argc = logic_ctx_.argc;
        logic_ctx.argv = logic_ctx_.argv;
        logic_ctx.common_component_dir = logic_ctx_.common_component_dir;
        logic_ctx.cur_working_dir = logic_ctx_.cur_working_dir;
        logic_ctx.app_name = logic_ctx_.app_name;
        logic_ctx.conf_center = logic_ctx_.conf_center;
        logic_ctx.timer_axis = logic_ctx_.timer_axis;
        logic_ctx.scheduler = &scheduler_;
        logic_ctx.msg_dispatcher = &msg_dispatcher_;
        logic_ctx.common_logic = burden_common_logic_;
        logic_ctx.thread_ev_base = logic_ctx_.thread_ev_base;
        logic_ctx.thread_idx = logic_ctx_.thread_idx;

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_ALWAYS("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}
}
