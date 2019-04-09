#include "io_thread_sink.h"
#include <unistd.h>
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "str_util.h"
#include "task_type.h"

namespace tcp
{
namespace raw
{
IOThreadSink::IOThreadSink() : common_logic_loader_(), logic_item_vec_(), conn_center_(),
    msg_codec_(), scheduler_(), msg_dispatcher_()
{
    threads_ctx_ = nullptr;
    listen_thread_ = nullptr;
    io_thread_group_ = nullptr;
    related_thread_group_ = nullptr;
    common_logic_ = nullptr;
}

IOThreadSink::~IOThreadSink()
{
}

void IOThreadSink::Release()
{
    for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_RELEASE_MODULE(it->logic, it->logic_loader);
    }

    logic_item_vec_.clear();
    SAFE_RELEASE_MODULE(common_logic_, common_logic_loader_);
    conn_center_.Release();

    delete this;
}

int IOThreadSink::OnInitialize(ThreadInterface* thread, const void* ctx)
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

    conn_center_.SetThreadSink(this);

    ConnCenterCtx conn_mgr_ctx;
    conn_mgr_ctx.timer_axis = self_thread_->GetTimerAxis();

    conn_mgr_ctx.inactive_conn_check_interval =
    {
        threads_ctx_->app_frame_conf_mgr->GetTCPInactiveConnCheckIntervalSec(),
        threads_ctx_->app_frame_conf_mgr->GetTCPInactiveConnCheckIntervalUsec()
    };

    conn_mgr_ctx.inactive_conn_life = threads_ctx_->app_frame_conf_mgr->GetTCPInactiveConnLife();
    conn_mgr_ctx.storm_interval = threads_ctx_->app_frame_conf_mgr->GetTCPStormInterval();
    conn_mgr_ctx.storm_threshold = threads_ctx_->app_frame_conf_mgr->GetTCPStormThreshold();

    if (conn_center_.Initialize(&conn_mgr_ctx) != 0)
    {
        return -1;
    }

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

void IOThreadSink::OnFinalize()
{
    for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_FINALIZE(it->logic);
    }

    SAFE_FINALIZE(common_logic_);
    scheduler_.Finalize();
    conn_center_.Finalize();

    ThreadSinkInterface::OnFinalize();
}

int IOThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    if (conn_center_.Activate() != 0)
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(common_logic_))
    {
        return -1;
    }

    for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        if (SAFE_ACTIVATE_FAILED(it->logic))
        {
            return -1;
        }
    }

    return 0;
}

void IOThreadSink::OnFreeze()
{
    for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_FREEZE(it->logic);
    }

    SAFE_FREEZE(common_logic_);
    conn_center_.Freeze();
    ThreadSinkInterface::OnFreeze();
}

void IOThreadSink::OnThreadStartOK()
{
    ThreadSinkInterface::OnThreadStartOK();

    pthread_mutex_lock(threads_ctx_->app_frame_threads_sync_mutex);
    ++(*threads_ctx_->app_frame_threads_count);
    pthread_cond_signal(threads_ctx_->app_frame_threads_sync_cond);
    pthread_mutex_unlock(threads_ctx_->app_frame_threads_sync_mutex);
}

void IOThreadSink::OnStop()
{
    ThreadSinkInterface::OnStop();

    if (common_logic_ != nullptr)
    {
        common_logic_->OnStop();
    }

    for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnStop();
    }
}

void IOThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    if (common_logic_ != nullptr)
    {
        common_logic_->OnReload();
    }

    for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnReload();
    }
}

void IOThreadSink::OnTask(const ThreadTask* task)
{
    ThreadSinkInterface::OnTask(task);

    switch (task->GetType())
    {
        case TASK_TYPE_TCP_CONN_CONNECTED:
        {
            NewConnCtx new_conn_ctx;
            memcpy(&new_conn_ctx, task->GetData().data(), task->GetData().size());
            OnClientConnected(&new_conn_ctx);
        }
        break;

        case TASK_TYPE_SEND_TO_CLIENT:
        {
            scheduler_.SendToClient(task->GetConnGUID(), task->GetData().data(), task->GetData().size());
        }
        break;

        case TASK_TYPE_CLOSE_CONN:
        {
            scheduler_.CloseClient(task->GetConnGUID());
        }
        break;

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
            }
            else
            {
                if (common_logic_ != nullptr)
                {
                    common_logic_->OnTask(task->GetConnGUID(), task->GetSourceThread(),
                                          task->GetData().data(), task->GetData().size());
                }

                for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
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

bool IOThreadSink::CanExit() const
{
    int can_exit = 1;

    if (common_logic_ != nullptr)
    {
        can_exit &= (common_logic_->CanExit() ? 1 : 0);
    }

    for (ProtoLogicItemVec::const_iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        can_exit &= (it->logic->CanExit() ? 1 : 0);
    }

    return (can_exit != 0);
}

void IOThreadSink::OnClientClosed(const BaseConn* conn, int task_type)
{
    if (common_logic_ != nullptr)
    {
        common_logic_->OnClientClosed(conn->GetConnGUID());
    }

    for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientClosed(conn->GetConnGUID());
    }

    char client_ctx_buf[128] = "";
    const int n = StrPrintf(client_ctx_buf, sizeof(client_ctx_buf), "%s:%u, socket fd: %d",
                            conn->GetClientIP(), conn->GetClientPort(), conn->GetSockFD());

    ThreadTask* task = new ThreadTask(task_type, self_thread_, nullptr, client_ctx_buf, n);
    if (nullptr == task)
    {
        LOG_ERROR("failed to create tcp conn closed task");
        return;
    }

    listen_thread_->PushTask(task);
    conn_center_.DestroyConn(conn->GetSockFD());
}

void IOThreadSink::OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len)
{
    if (common_logic_ != nullptr)
    {
        common_logic_->OnRecvClientData(conn_guid, data, len);
    }

    for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnRecvClientData(conn_guid, data, len);
    }
}

void IOThreadSink::SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups)
{
    related_thread_group_ = related_thread_groups;

    if (related_thread_group_->global_logic != nullptr)
    {
        if (common_logic_ != nullptr)
        {
            common_logic_->SetGlobalLogic(related_thread_group_->global_logic);
        }

        for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
        {
            ProtoLogicItem& logic_item = *it;
            logic_item.logic->SetGlobalLogic(related_thread_group_->global_logic);
        }
    }

    scheduler_.SetRelatedThreadGroups(related_thread_groups);
}

int IOThreadSink::LoadCommonLogic()
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
    logic_ctx.conn_center = &conn_center_;
    logic_ctx.scheduler = &scheduler_;
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

int IOThreadSink::LoadLogicGroup()
{
    // logic so group
    const StrGroup& logic_so_group = threads_ctx_->conf.logic_so_group;

    for (StrGroup::const_iterator it = logic_so_group.begin(); it != logic_so_group.end(); ++it)
    {
        char logic_so_path[MAX_PATH_LEN] = "";
        GetAbsolutePath(logic_so_path, sizeof(logic_so_path), (*it).c_str(), threads_ctx_->cur_working_dir);

        ProtoLogicItem logic_item;
        logic_item.logic_so_path = logic_so_path;
        logic_item.logic = nullptr;

        logic_item_vec_.push_back(logic_item);
    }

    for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        ProtoLogicItem& logic_item = *it;
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
        logic_ctx.timer_axis = self_thread_->GetTimerAxis();
        logic_ctx.conn_center = &conn_center_;
        logic_ctx.scheduler = &scheduler_;
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

int IOThreadSink::OnClientConnected(const NewConnCtx* new_conn_ctx)
{
    BaseConn* conn = static_cast<BaseConn*>(conn_center_.GetConnBySockFD(new_conn_ctx->client_sock_fd));
    if (conn != nullptr)
    {
        LOG_WARN("tcp conn already exist, socket fd: " << new_conn_ctx->client_sock_fd << ", destroy it first");
        conn_center_.DestroyConn(conn->GetSockFD());
    }

    conn = conn_center_.CreateConn(threads_ctx_->conf.io_type, self_thread_->GetThreadIdx(), new_conn_ctx->client_ip,
                                   new_conn_ctx->client_port, new_conn_ctx->client_sock_fd);
    if (nullptr == conn)
    {
        char client_ctx_buf[128] = "";
        const int n = StrPrintf(client_ctx_buf, sizeof(client_ctx_buf), "%s:%u, socket fd: %d",
                                new_conn_ctx->client_ip, new_conn_ctx->client_port,
                                new_conn_ctx->client_sock_fd);

        ThreadTask* task = new ThreadTask(TASK_TYPE_TCP_CONN_CLOSED, self_thread_, nullptr, client_ctx_buf, n);
        if (nullptr == task)
        {
            LOG_ERROR("failed to create tcp conn closed task");
            return -1;
        }

        listen_thread_->PushTask(task);
        return -1;
    }

    if (common_logic_ != nullptr)
    {
        common_logic_->OnClientConnected(conn->GetConnGUID());
    }

    for (ProtoLogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientConnected(conn->GetConnGUID());
    }

    return 0;
}
}
}
