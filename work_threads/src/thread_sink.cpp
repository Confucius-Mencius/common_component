#include "thread_sink.h"
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "str_util.h"
#include "task_type.h"

namespace work
{
ThreadSink::ThreadSink() : common_logic_loader_(), logic_item_vec_(), msg_codec_(), scheduler_(), msg_dispatcher_(),
    trans_center_loader_(), client_center_mgr_loader_()
{
    threads_ctx_ = nullptr;
    work_thread_group_ = nullptr;
    common_logic_ = nullptr;
    trans_center_ = nullptr;
    client_center_mgr_ = nullptr;
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
    SAFE_RELEASE_MODULE(client_center_mgr_, client_center_mgr_loader_);
    SAFE_RELEASE_MODULE(trans_center_, trans_center_loader_);

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

    if (LoadTransCenter() != 0)
    {
        return -1;
    }

    if (LoadClientCenterMgr() != 0)
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
    SAFE_FINALIZE(client_center_mgr_);
    SAFE_FINALIZE(trans_center_);

    scheduler_.Finalize();
    ThreadSinkInterface::OnFinalize();
}

int ThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(trans_center_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(client_center_mgr_))
    {
        return -1;
    }

    tcp::proto::ClientCenterCtx proto_tcp_client_center_ctx;
    proto_tcp_client_center_ctx.thread_ev_base = self_thread_->GetThreadEvBase();
    proto_tcp_client_center_ctx.msg_codec = &msg_codec_;
    proto_tcp_client_center_ctx.timer_axis = self_thread_->GetTimerAxis();
    proto_tcp_client_center_ctx.trans_center = trans_center_;
    proto_tcp_client_center_ctx.reconnect_interval =
    {
        threads_ctx_->app_frame_conf_mgr->GetPeerProtoTCPReconnIntervalSec(),
        threads_ctx_->app_frame_conf_mgr->GetPeerProtoTCPReconnIntervalUsec()
    };

    proto_tcp_client_center_ctx.reconnect_limit = threads_ctx_->app_frame_conf_mgr->GetPeerProtoTCPReconnLimit();

    proto_tcp_client_center_ = client_center_mgr_->CreateProtoTCPClientCenter(&proto_tcp_client_center_ctx);
    if (nullptr == proto_tcp_client_center_)
    {
        return -1;
    }

    http::ClientCenterCtx http_client_center_ctx;
    http_client_center_ctx.thread_ev_base = self_thread_->GetThreadEvBase();
    http_client_center_ctx.timer_axis = self_thread_->GetTimerAxis();
    http_client_center_ctx.trans_center = trans_center_;
    http_client_center_ctx.http_conn_timeout = threads_ctx_->app_frame_conf_mgr->GetPeerHTTPConnTimeout();
    http_client_center_ctx.http_conn_max_retry = threads_ctx_->app_frame_conf_mgr->GetPeerHTTPConnMaxRetry();

    http_client_center_ = client_center_mgr_->CreateHTTPClientCenter(&http_client_center_ctx);
    if (nullptr == http_client_center_)
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
    SAFE_FREEZE(client_center_mgr_);
    SAFE_FREEZE(trans_center_);

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
                // work threads自己的消息分发
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
            else
            {
                // 基于work threads的线程组自己实现的消息分发
                for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
                {
                    it->logic->OnTask(task->GetConnGUID(), task->GetSourceThread(),
                                      task->GetData().data(), task->GetData().size());
                }
            }
        }
        break;

        default:
        {
            LOG_ERROR("invalid task type: " << task->GetType());
            return;
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

    for (LogicItemVec::const_iterator it = logic_item_vec_.cbegin(); it != logic_item_vec_.cend(); ++it)
    {
        can_exit &= (it->logic->CanExit() ? 1 : 0);
    }

    return (can_exit != 0);
}

void ThreadSink::OnRecvNfy(const Peer& peer, const proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
{
    if (0 == msg_dispatcher_.DispatchMsg(nullptr, msg_head, msg_body, msg_body_len))
    {
        LOG_TRACE("dispatch msg ok, msg id: " << msg_head.msg_id);
        return;
    }

    LOG_ERROR("failed to dispatch msg, msg id: " << msg_head.msg_id);
}

int ThreadSink::LoadCommonLogic()
{
    const std::string& common_logic_so = threads_ctx_->conf.common_logic_so;
    if (0 == common_logic_so.length())
    {
        return 0;
    }

    char common_logic_so_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(common_logic_so_path, sizeof(common_logic_so_path),
                    common_logic_so.c_str(), threads_ctx_->cur_working_dir);
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
    logic_ctx.msg_codec = &msg_codec_;
    logic_ctx.msg_dispatcher = &msg_dispatcher_;
    logic_ctx.common_logic = common_logic_;
    logic_ctx.thread_ev_base = self_thread_->GetThreadEvBase();
    logic_ctx.thread_idx = self_thread_->GetThreadIdx();
    logic_ctx.logic_args = threads_ctx_->logic_args;

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
    const StrGroup& logic_so_group = threads_ctx_->conf.logic_so_group;

    for (StrGroup::const_iterator it = logic_so_group.cbegin(); it != logic_so_group.cend(); ++it)
    {
        char logic_so_path[MAX_PATH_LEN] = "";
        GetAbsolutePath(logic_so_path, sizeof(logic_so_path), (*it).c_str(), threads_ctx_->cur_working_dir);

        LogicItem logic_item;
        logic_item.logic_so_path = logic_so_path;
        logic_item.logic = nullptr;

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
        logic_ctx.msg_codec = &msg_codec_;
        logic_ctx.msg_dispatcher = &msg_dispatcher_;
        logic_ctx.common_logic = common_logic_;
        logic_ctx.thread_ev_base = self_thread_->GetThreadEvBase();
        logic_ctx.thread_idx = self_thread_->GetThreadIdx();
        logic_ctx.logic_args = threads_ctx_->logic_args;

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_ALWAYS("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}

int ThreadSink::LoadTransCenter()
{
    char trans_center_so_path[MAX_PATH_LEN + 1] = "";
    StrPrintf(trans_center_so_path, sizeof(trans_center_so_path), "%s/libtrans_center.so",
              threads_ctx_->common_component_dir);

    if (trans_center_loader_.Load(trans_center_so_path) != 0)
    {
        LOG_ERROR(trans_center_loader_.GetLastErrMsg());
        return -1;
    }

    trans_center_ = (TransCenterInterface*) trans_center_loader_.GetModuleInterface();
    if (nullptr == trans_center_)
    {
        LOG_ERROR(trans_center_loader_.GetLastErrMsg());
        return -1;
    }

    TransCenterCtx trans_center_ctx;
    trans_center_ctx.timer_axis = self_thread_->GetTimerAxis();
    trans_center_ctx.rsp_check_interval = threads_ctx_->app_frame_conf_mgr->GetPeerRspCheckInterval();

    if (trans_center_->Initialize(&trans_center_ctx) != 0)
    {
        LOG_ERROR("failed to initialize trans center");
        return -1;
    }

    return 0;
}

int ThreadSink::LoadClientCenterMgr()
{
    char client_center_mgr_so_path[MAX_PATH_LEN + 1] = "";
    StrPrintf(client_center_mgr_so_path, sizeof(client_center_mgr_so_path), "%s/libclient_center_mgr.so",
              threads_ctx_->common_component_dir);

    if (client_center_mgr_loader_.Load(client_center_mgr_so_path) != 0)
    {
        LOG_ERROR(client_center_mgr_loader_.GetLastErrMsg());
        return -1;
    }

    client_center_mgr_ = (ClientCenterMgrInterface*) client_center_mgr_loader_.GetModuleInterface();
    if (nullptr == client_center_mgr_)
    {
        LOG_ERROR(client_center_mgr_loader_.GetLastErrMsg());
        return -1;
    }

    if (client_center_mgr_->Initialize(nullptr) != 0)
    {
        LOG_ERROR("failed to initialize client center mgr");
        return -1;
    }

    return 0;
}
}
