#include "thread_sink.h"
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "str_util.h"
#include "task_type.h"

namespace global
{
ThreadSink::ThreadSink() : logic_loader_(), msg_codec_(), scheduler_(), msg_dispatcher_()
{
    threads_ctx_ = nullptr;
    logic_ = nullptr;
    reload_finished_ = false;
}

ThreadSink::~ThreadSink()
{
}

void ThreadSink::Release()
{
    SAFE_RELEASE_MODULE(logic_, logic_loader_);
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

    if (LoadLogic() != 0)
    {
        return -1;
    }

    return 0;
}

void ThreadSink::OnFinalize()
{
    SAFE_FINALIZE(logic_);
    scheduler_.Finalize();
    ThreadSinkInterface::OnFinalize();
}

int ThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(logic_))
    {
        return -1;
    }

    return 0;
}

void ThreadSink::OnFreeze()
{
    SAFE_FREEZE(logic_);
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

    if (logic_ != nullptr)
    {
        logic_->OnStop();
    }
}

void ThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    if (logic_ != nullptr)
    {
        logic_->OnReload();
    }

    reload_finished_ = true;
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
                    LOG_TRACE("dispatch msg ok, " << conn_guid << ", msg id: " << msg_head.msg_id);
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
    if (logic_ != nullptr)
    {
        return logic_->CanExit();
    }

    return true;
}

int ThreadSink::LoadLogic()
{
    const std::string logic_so = threads_ctx_->app_frame_conf_mgr->GetGlobalLogicSo();
    if (0 == logic_so.length())
    {
        return 0;
    }

    char logic_so_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(logic_so_path, sizeof(logic_so_path),
                    logic_so.c_str(), threads_ctx_->cur_working_dir);
    LOG_ALWAYS("load logic so " << logic_so_path << " begin");

    if (logic_loader_.Load(logic_so_path) != 0)
    {
        LOG_ERROR("failed to load logic so, " << logic_loader_.GetLastErrMsg());
        return -1;
    }

    logic_ = static_cast<LogicInterface*>(logic_loader_.GetModuleInterface(0));
    if (nullptr == logic_)
    {
        LOG_ERROR("failed to get logic, " << logic_loader_.GetLastErrMsg());
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
    logic_ctx.global_logic = logic_;
    logic_ctx.thread_ev_base = self_thread_->GetThreadEvBase();
    
    if (logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_ALWAYS("load logic so " << logic_so_path << " end");
    return 0;
}
}
