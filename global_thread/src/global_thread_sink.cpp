#include "global_thread_sink.h"

#if defined(NDEBUG)
#include <gperftools/profiler.h>
#endif

#include "app_frame_conf_mgr_interface.h"
#include "msg_codec_center_interface.h"
#include "msg_codec_interface.h"
#include "msg_handler_interface.h"
#include "str_util.h"

namespace global
{
ThreadSink::ThreadSink() : scheduler_(), logic_loader_(), reload_finished_mutex_()
{
    threads_ctx_ = NULL;
    tcp_client_center_ = NULL;
    http_client_center_ = NULL;
    udp_client_center_ = NULL;
    logic_ = NULL;
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

int ThreadSink::OnInitialize(ThreadInterface* thread)
{
    if (ThreadSinkInterface::OnInitialize(thread) != 0)
    {
        return -1;
    }

    tcp::ClientCenterCtx tcp_client_center_ctx;
    tcp_client_center_ctx.thread_ev_base = thread_->GetThreadEvBase();

    {
        MsgCodecCtx msg_codec_ctx;
        msg_codec_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetTcpMaxMsgBodyLen();
        msg_codec_ctx.do_checksum = threads_ctx_->conf_mgr->TcpDoChecksum();

        MsgCodecInterface* msg_codec_ = threads_ctx_->msg_codec_center->CreateMsgCodec(&msg_codec_ctx);
        if (NULL == msg_codec_)
        {
            LOG_ERROR("failed to create tcp msg codec");
            return -1;
        }

        tcp_client_center_ctx.msg_codec = msg_codec_;
    }

    tcp_client_center_ctx.timer_axis = thread_->GetTimerAxis();
    tcp_client_center_ctx.trans_center = thread_->GetTransCenter();
    tcp_client_center_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetTcpMaxMsgBodyLen();
    tcp_client_center_ctx.reconnect_interval = {threads_ctx_->conf_mgr->GetPeerTcpConnIntervalSec(),
                                                threads_ctx_->conf_mgr->GetPeerTcpConnIntervalUsec()
                                               };

    tcp_client_center_ = thread_->GetClientCenterMgr()->CreateTcpClientCenter(&tcp_client_center_ctx);
    if (NULL == tcp_client_center_)
    {
        return -1;
    }

    http::ClientCenterCtx http_client_center_ctx;
    http_client_center_ctx.thread_ev_base = thread_->GetThreadEvBase();
    http_client_center_ctx.timer_axis = thread_->GetTimerAxis();
    http_client_center_ctx.trans_center = thread_->GetTransCenter();
    http_client_center_ctx.http_conn_timeout = threads_ctx_->conf_mgr->GetPeerHttpConnTimeout();
    http_client_center_ctx.http_conn_max_retry = threads_ctx_->conf_mgr->GetPeerHttpConnMaxRetry();

    http_client_center_ = thread_->GetClientCenterMgr()->CreateHttpClientCenter(&http_client_center_ctx);
    if (NULL == http_client_center_)
    {
        return -1;
    }

    udp::ClientCenterCtx udp_client_center_ctx;
    udp_client_center_ctx.thread_ev_base = thread_->GetThreadEvBase();

    {
        MsgCodecCtx msg_codec_ctx;
        msg_codec_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetUdpMaxMsgBodyLen();
        msg_codec_ctx.do_checksum = threads_ctx_->conf_mgr->UdpDoChecksum();

        MsgCodecInterface* msg_codec_ = threads_ctx_->msg_codec_center->CreateMsgCodec(&msg_codec_ctx);
        if (NULL == msg_codec_)
        {
            LOG_ERROR("failed to create tcp msg codec");
            return -1;
        }

        udp_client_center_ctx.msg_codec = msg_codec_;
    }

    udp_client_center_ctx.timer_axis = thread_->GetTimerAxis();
    udp_client_center_ctx.trans_center = thread_->GetTransCenter();
    udp_client_center_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetUdpMaxMsgBodyLen();

    udp_client_center_ = thread_->GetClientCenterMgr()->CreateUdpClientCenter(&udp_client_center_ctx);
    if (NULL == udp_client_center_)
    {
        return -1;
    }

    scheduler_.SetGlobalThreadSink(this);

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

#if defined(NDEBUG)
    if (threads_ctx_->conf_mgr->EnableCpuProfiling())
    {
        LOG_INFO("enable cpu profiling");
        ProfilerRegisterThread();
    }
#endif

    pthread_mutex_lock(threads_ctx_->frame_threads_mutex);
    ++(*threads_ctx_->frame_threads_count);
    pthread_cond_signal(threads_ctx_->frame_threads_cond);
    pthread_mutex_unlock(threads_ctx_->frame_threads_mutex);
}

void ThreadSink::OnStop()
{
    ThreadSinkInterface::OnStop();

    if (logic_ != NULL)
    {
        logic_->OnStop();
    }
}

void ThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    if (logic_ != NULL)
    {
        logic_->OnReload();
    }

    reload_finished_ = true;
}

void ThreadSink::OnTask(const Task* task)
{
    ThreadSinkInterface::OnTask(task);
    const TaskCtx* task_ctx = task->GetCtx();

    switch (task_ctx->task_type)
    {
        case TASK_TYPE_NORMAL:
        {
            if (0 == thread_->GetMsgDispatcher()->DispatchMsg(&task_ctx->conn_guid, task_ctx->msg_head,
                    task_ctx->msg_body, task_ctx->msg_body_len))
            {
                LOG_TRACE("dispatch msg ok, " << task_ctx->conn_guid << ", msg id: "
                          << task_ctx->msg_head.msg_id);
                return;
            }

            LOG_ERROR("failed to dispatch msg, msg id: " << task_ctx->msg_head.msg_id);
        }
        break;

        default:
        {
            LOG_ERROR("invalid task type: " << task_ctx->task_type);
        }
        break;
    }
}

bool ThreadSink::CanExit() const
{
    if (logic_ != NULL)
    {
        return logic_->CanExit();
    }

    return true;
}

void ThreadSink::OnRecvNfy(const Peer& peer, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
{
    if (0 == thread_->GetMsgDispatcher()->DispatchMsg(NULL, msg_head, msg_body, msg_body_len))
    {
        LOG_TRACE("dispatch msg ok, msg id: " << msg_head.msg_id);
        return;
    }

    LOG_ERROR("failed to dispatch msg, msg id: " << msg_head.msg_id);
}

int ThreadSink::LoadLogic()
{
    const std::string logic_so_path = GetAbsolutePath(threads_ctx_->conf_mgr->GetGlobalLogicSo().c_str(),
                                      threads_ctx_->cur_work_dir);
    LOG_INFO("load logic so " << logic_so_path << " begin");

    if (logic_loader_.Load(logic_so_path.c_str()) != 0)
    {
        LOG_ERROR("failed to load logic so, " << logic_loader_.GetLastErrMsg());
        return -1;
    }

    logic_ = (LogicInterface*) logic_loader_.GetModuleInterface(0);
    if (NULL == logic_)
    {
        LOG_ERROR("failed to get logic interface, " << logic_loader_.GetLastErrMsg());
        return -1;
    }

    LOG_INFO("load logic so " << logic_so_path << " ...");

    LogicCtx logic_ctx;
    logic_ctx.argc = threads_ctx_->argc;
    logic_ctx.argv = threads_ctx_->argv;
    logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
    logic_ctx.cur_work_dir = threads_ctx_->cur_work_dir;
    logic_ctx.app_name = threads_ctx_->app_name;
    logic_ctx.conf_center = threads_ctx_->conf_center;
    logic_ctx.timer_axis = thread_->GetTimerAxis();
    logic_ctx.time_service = thread_->GetTimeService();
    logic_ctx.random_engine = thread_->GetRandomEngine();
    logic_ctx.msg_dispatcher = thread_->GetMsgDispatcher();
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.global_logic = logic_;

    if (logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_INFO("load logic so " << logic_so_path << " end");
    return 0;
}
}
