#include "burden_thread_sink.h"

#if defined(NDEBUG)
#include <gperftools/profiler.h>
#endif

#include "app_frame_conf_mgr_interface.h"
#include "msg_codec_center_interface.h"
#include "msg_codec_interface.h"
#include "msg_handler_interface.h"
#include "str_util.h"

namespace burden
{
ThreadSink::ThreadSink() : scheduler_(), local_logic_loader_(), logic_item_vec_()
{
    threads_ctx_ = NULL;
    burden_thread_group_ = NULL;
    tcp_client_center_ = NULL;
    http_client_center_ = NULL;
    udp_client_center_ = NULL;
    local_logic_ = NULL;
}

ThreadSink::~ThreadSink()
{
}

void ThreadSink::Release()
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_RELEASE_MODULE(it->logic, it->logic_loader);
    }

    logic_item_vec_.clear();

    SAFE_RELEASE_MODULE(local_logic_, local_logic_loader_);
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
                                                threads_ctx_->conf_mgr->GetPeerTcpConnIntervalUsec()};

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

    scheduler_.SetThreadSink(this);

    if (scheduler_.Initialize(threads_ctx_) != 0)
    {
        return -1;
    }

    if (LoadLocalLogic() != 0)
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

    SAFE_FINALIZE(local_logic_);
    scheduler_.Finalize();
    ThreadSinkInterface::OnFinalize();
}

int ThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(local_logic_))
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

    SAFE_FREEZE(local_logic_);
    ThreadSinkInterface::OnFreeze();
}

void ThreadSink::OnThreadStartOk()
{
    ThreadSinkInterface::OnThreadStartOk();

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

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnStop();
    }

    if (local_logic_ != NULL)
    {
        local_logic_->OnStop();
    }
}

void ThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnReload();
    }

    if (local_logic_ != NULL)
    {
        local_logic_->OnReload();
    }
}

void ThreadSink::OnTask(const Task* task)
{
    ThreadSinkInterface::OnTask(task);
    const TaskCtx* task_ctx = task->GetCtx();

    switch (task_ctx->task_type)
    {
        case TASK_TYPE_NORMAL:
        {
            if (logic_item_vec_.size() > 0)
            {
                if (0 == thread_->GetMsgDispatcher()->DispatchMsg(&task_ctx->conn_guid, task_ctx->msg_head,
                                                                  task_ctx->msg_body, task_ctx->msg_body_len))
                {
                    LOG_TRACE("dispatch msg ok, " << task_ctx->conn_guid << ", msg id: "
                                  << task_ctx->msg_head.msg_id);
                    return;
                }
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
    int can_exit = 1;

    for (LogicItemVec::const_iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        can_exit &= (it->logic->CanExit() ? 1 : 0);
    }

    if (local_logic_ != NULL)
    {
        can_exit &= (local_logic_->CanExit() ? 1 : 0);
    }

    return (can_exit != 0);
}

void ThreadSink::OnRecvNfy(const Peer& peer, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
{
    if (logic_item_vec_.size() > 0)
    {
        if (0 == thread_->GetMsgDispatcher()->DispatchMsg(NULL, msg_head, msg_body, msg_body_len))
        {
            LOG_TRACE("dispatch msg ok, msg id: " << msg_head.msg_id);
            return;
        }
    }

    LOG_ERROR("failed to dispatch msg, msg id: " << msg_head.msg_id);
}

void ThreadSink::SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group)
{
    if (related_thread_group->global_logic != NULL)
    {
        if (local_logic_ != NULL)
        {
            local_logic_->SetGlobalLogic(related_thread_group->global_logic);
        }

        for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
        {
            LogicItem& logic_item = *it;
            logic_item.logic->SetGlobalLogic(related_thread_group->global_logic);
        }
    }

    scheduler_.SetRelatedThreadGroup(related_thread_group);
}

int ThreadSink::LoadLocalLogic()
{
    if (0 == threads_ctx_->conf_mgr->GetBurdenLocalLogicSo().length())
    {
        return 0;
    }

    const std::string local_logic_so_path = GetAbsolutePath(threads_ctx_->conf_mgr->GetBurdenLocalLogicSo().c_str(),
                                                            threads_ctx_->cur_work_dir);
    LOG_INFO("load local logic so " << local_logic_so_path << " begin");

    if (local_logic_loader_.Load(local_logic_so_path.c_str()) != 0)
    {
        LOG_ERROR("failed to load local logic so " << local_logic_so_path
                      << ", " << local_logic_loader_.GetLastErrMsg());
        return -1;
    }

    local_logic_ = (LocalLogicInterface*) local_logic_loader_.GetModuleInterface();
    if (NULL == local_logic_)
    {
        LOG_ERROR("failed to get local logic interface, " << local_logic_loader_.GetLastErrMsg());
        return -1;
    }

    LOG_INFO("load local logic so " << local_logic_so_path << " ...");

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
    logic_ctx.msg_dispatcher = NULL;
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.local_logic = local_logic_;

    if (local_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_INFO("load local logic so " << local_logic_so_path << " end");
    return 0;
}

int ThreadSink::LoadLogicGroup()
{
    // logic group
    LogicItem logic_item;
    logic_item.logic = NULL;

    const StrGroup logic_so_group = threads_ctx_->conf_mgr->GetBurdenLogicSoGroup();

    for (StrGroup::const_iterator it = logic_so_group.begin();
         it != logic_so_group.end(); ++it)
    {
        logic_item.logic_so_path = GetAbsolutePath((*it).c_str(), threads_ctx_->cur_work_dir);
        logic_item_vec_.push_back(logic_item);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        LogicItem& logic_item = *it;
        LOG_INFO("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so " << logic_item.logic_so_path << ", "
                          << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = (LogicInterface*) logic_item.logic_loader.GetModuleInterface();
        if (NULL == logic_item.logic)
        {
            LOG_ERROR("failed to get logic interface, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        LOG_INFO("load logic so " << logic_item.logic_so_path << " ...");

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
        logic_ctx.local_logic = local_logic_;

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_INFO("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}
}
