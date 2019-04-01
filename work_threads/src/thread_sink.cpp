#include "thread_sink.h"
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "str_util.h"
#include "task_type.h"

namespace work
{
ThreadSink::ThreadSink() : common_logic_loader_(), logic_item_vec_(), msg_codec_(), scheduler_(), msg_dispatcher_()
{
    threads_ctx_ = nullptr;
    work_thread_group_ = nullptr;
    common_logic_ = nullptr;
}

ThreadSink::~ThreadSink()
{
}

void ThreadSink::SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups)
{
    if (related_thread_groups->global_logic != nullptr)
    {
        if (common_logic_ != nullptr)
        {
            common_logic_->SetGlobalLogic(related_thread_groups->global_logic);
        }

        for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
        {
            LogicItem& logic_item = *it;
            logic_item.logic->SetGlobalLogic(related_thread_groups->global_logic);
        }
    }

    scheduler_.SetRelatedThreadGroups(related_thread_groups);
}

void ThreadSink::Release()
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_RELEASE_MODULE(it->logic, it->logic_loader);
    }

    logic_item_vec_.clear();
    SAFE_RELEASE_MODULE(common_logic_, common_logic_loader_);

    delete this;
}

int ThreadSink::OnInitialize(ThreadInterface* thread, const void* ctx)
{
    if (ThreadSinkInterface::OnInitialize(thread, ctx) != 0)
    {
        return -1;
    }

    threads_ctx_ = static_cast<const ThreadsCtx*>(ctx);

    ::proto::MsgCodecCtx msg_codec_ctx;
    msg_codec_ctx.max_msg_body_len = threads_ctx_->app_frame_conf_mgr->GetProtoMaxMsgBodyLen();
    msg_codec_ctx.do_checksum = threads_ctx_->app_frame_conf_mgr->ProtoDoChecksum();
    msg_codec_.SetCtx(&msg_codec_ctx);

    scheduler_.SetThreadSink(this);
    scheduler_.SetMsgCodec(&msg_codec_);

    if (scheduler_.Initialize(threads_ctx_) != 0)
    {
        return -1;
    }

    if (LoadCommonLogic() != 0)
    {
        return -1;
    }

    if (LoadLogicGroup() != 0)
    {
        return -1;
    }

    return 0;
}

void ThreadSink::OnFinalize()
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_FINALIZE(it->logic);
    }

    SAFE_FINALIZE(common_logic_);
    scheduler_.Finalize();
    ThreadSinkInterface::OnFinalize();
}

int ThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(common_logic_))
    {
        return -1;
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        if (SAFE_ACTIVATE_FAILED(it->logic))
        {
            return -1;
        }
    }

    return 0;
}

void ThreadSink::OnFreeze()
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_FREEZE(it->logic);
    }

    SAFE_FREEZE(common_logic_);
    ThreadSinkInterface::OnFreeze();
}

void ThreadSink::OnThreadStartOK()
{
    ThreadSinkInterface::OnThreadStartOK();

    pthread_mutex_lock(threads_ctx_->app_frame_threads_sync_mutex);
    ++(*threads_ctx_->app_frame_threads_count);
    pthread_cond_signal(threads_ctx_->app_frame_threads_sync_cond);
    pthread_mutex_unlock(threads_ctx_->app_frame_threads_sync_mutex);
}

void ThreadSink::OnStop()
{
    ThreadSinkInterface::OnStop();

    if (common_logic_ != nullptr)
    {
        common_logic_->OnStop();
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnStop();
    }
}

void ThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    if (common_logic_ != nullptr)
    {
        common_logic_->OnReload();
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnReload();
    }
}

void ThreadSink::OnTask(const ThreadTask* task)
{
    ThreadSinkInterface::OnTask(task);

    switch (task->GetType())
    {
        case TASK_TYPE_NORMAL:
        {
            const char* data = task->GetData().data();
            size_t len = task->GetData().size();

            if (nullptr == data || 0 == len)
            {
                LOG_ERROR("invalid params");
                return;
            }

            ::proto::MsgID err_msg_id;
            ::proto::MsgHead msg_head;
            char* msg_body = nullptr;
            size_t msg_body_len = 0;

            if (msg_codec_.DecodeMsg(err_msg_id, &msg_head, &msg_body, msg_body_len, (const char*) data, len) != 0)
            {
                return;
            }

            const ConnGUID* conn_guid = task->GetConnGUID();
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

                return;
            }

            LOG_ERROR("failed to dispatch msg, msg id: " << msg_head.msg_id);
        }
        break;

        default:
        {
            LOG_ERROR("invalid task type: " << task->GetType());
        }
        break;
    }
}

bool ThreadSink::CanExit() const
{
    int can_exit = 1;

    if (common_logic_ != nullptr)
    {
        can_exit &= (common_logic_->CanExit() ? 1 : 0);
    }

    for (LogicItemVec::const_iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        can_exit &= (it->logic->CanExit() ? 1 : 0);
    }

    return (can_exit != 0);
}

int ThreadSink::LoadCommonLogic()
{
    if (0 == threads_ctx_->app_frame_conf_mgr->GetWorkCommonLogicSo().length())
    {
        return 0;
    }

    char common_logic_so_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(common_logic_so_path, sizeof(common_logic_so_path),
                    threads_ctx_->app_frame_conf_mgr->GetWorkCommonLogicSo().c_str(), threads_ctx_->cur_working_dir);
    LOG_ALWAYS("load common logic so " << common_logic_so_path << " begin");

    if (common_logic_loader_.Load(common_logic_so_path) != 0)
    {
        LOG_ERROR("failed to load common logic so, " << common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    common_logic_ = static_cast<CommonLogicInterface*>(common_logic_loader_.GetModuleInterface());
    if (nullptr == common_logic_)
    {
        LOG_ERROR("failed to get common logic, " << common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    LogicCtx logic_ctx;
    logic_ctx.argc = threads_ctx_->argc;
    logic_ctx.argv = threads_ctx_->argv;
    logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
    logic_ctx.cur_working_dir = threads_ctx_->cur_working_dir;
    logic_ctx.app_name = threads_ctx_->app_name;
    logic_ctx.conf_center = threads_ctx_->conf_center;
    logic_ctx.timer_axis = self_thread_->GetTimerAxis();
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.msg_dispatcher = &msg_dispatcher_;
    logic_ctx.common_logic = common_logic_;

    if (common_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_ALWAYS("load common logic so " << common_logic_so_path << " end");
    return 0;
}

int ThreadSink::LoadLogicGroup()
{
    // logic group
    LogicItem logic_item;
    logic_item.logic = nullptr;

    const StrGroup logic_so_group = threads_ctx_->app_frame_conf_mgr->GetWorkLogicSoGroup();

    for (StrGroup::const_iterator it = logic_so_group.begin();
            it != logic_so_group.end(); ++it)
    {
        char logic_so_path[MAX_PATH_LEN] = "";
        GetAbsolutePath(logic_so_path, sizeof(logic_so_path), (*it).c_str(), threads_ctx_->cur_working_dir);
        logic_item.logic_so_path = logic_so_path;
        logic_item_vec_.push_back(logic_item);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        LogicItem& logic_item = *it;
        LOG_ALWAYS("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = static_cast<LogicInterface*>(logic_item.logic_loader.GetModuleInterface());
        if (nullptr == logic_item.logic)
        {
            LOG_ERROR("failed to get logic, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        LogicCtx logic_ctx;
        logic_ctx.argc = threads_ctx_->argc;
        logic_ctx.argv = threads_ctx_->argv;
        logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
        logic_ctx.cur_working_dir = threads_ctx_->cur_working_dir;
        logic_ctx.app_name = threads_ctx_->app_name;
        logic_ctx.conf_center = threads_ctx_->conf_center;
        logic_ctx.timer_axis = self_thread_->GetTimerAxis();;
        logic_ctx.scheduler = &scheduler_;
        logic_ctx.msg_dispatcher = &msg_dispatcher_;
        logic_ctx.common_logic = common_logic_;

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_ALWAYS("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}
}
