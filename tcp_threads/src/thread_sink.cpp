#include "thread_sink.h"
#include <unistd.h>
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "str_util.h"
#include "task_type.h"

namespace tcp
{
namespace raw
{
ThreadSink::ThreadSink()
    : common_logic_loader_(), logic_item_vec_(), conn_mgr_(), scheduler_()
{
    threads_ctx_ = NULL;
    listen_thread_ = NULL;
    tcp_thread_group_ = NULL;
    related_thread_group_ = NULL;
    common_logic_ = NULL;
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
    SAFE_RELEASE_MODULE(common_logic_, common_logic_loader_);
    conn_mgr_.Release();

    delete this;
}

int ThreadSink::OnInitialize(ThreadInterface* thread, const void* ctx)
{
    if (ThreadSinkInterface::OnInitialize(thread, ctx) != 0)
    {
        return -1;
    }

    threads_ctx_ = static_cast<const ThreadsCtx*>(ctx);

    conn_mgr_.SetThreadSink(this);
    scheduler_.SetThreadSink(this);

    ConnMgrCtx conn_mgr_ctx;
    conn_mgr_ctx.timer_axis = self_thread_->GetTimerAxis();

    conn_mgr_ctx.inactive_conn_check_interval =
    {
        threads_ctx_->conf_mgr->GetTCPInactiveConnCheckIntervalSec(),
        threads_ctx_->conf_mgr->GetTCPInactiveConnCheckIntervalUsec()
    };

    conn_mgr_ctx.inactive_conn_life = threads_ctx_->conf_mgr->GetTCPInactiveConnLife();
    conn_mgr_ctx.storm_interval = threads_ctx_->conf_mgr->GetTCPStormInterval();
    conn_mgr_ctx.storm_threshold = threads_ctx_->conf_mgr->GetTCPStormThreshold();

    if (conn_mgr_.Initialize(&conn_mgr_ctx) != 0)
    {
        return -1;
    }

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
    conn_mgr_.Finalize();

    ThreadSinkInterface::OnFinalize();
}

int ThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    if (conn_mgr_.Activate() != 0)
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
    conn_mgr_.Freeze();
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

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnStop();
    }

    if (common_logic_ != NULL)
    {
        common_logic_->OnStop();
    }
}

void ThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnReload();
    }

    if (common_logic_ != NULL)
    {
        common_logic_->OnReload();
    }
}

void ThreadSink::OnTask(const ThreadTask* task)
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

        case TASK_TYPE_TCP_SEND_TO_CLIENT:
        {
            scheduler_.SendToClient(task->GetConnGUID(), task->GetData().data(), task->GetData().size());
        }
        break;

        case TASK_TYPE_TCP_CLOSE_CONN:
        {
            scheduler_.CloseClient(task->GetConnGUID());
        }
        break;

        case TASK_TYPE_NORMAL:
        {
            for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
            {
                it->logic->OnTask(task->GetConnGUID(), task->GetSourceThread(),
                                  task->GetData().data(), task->GetData().size());
            }
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

    for (LogicItemVec::const_iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        can_exit &= (it->logic->CanExit() ? 1 : 0);
    }

    if (common_logic_ != NULL)
    {
        can_exit &= (common_logic_->CanExit() ? 1 : 0);
    }

    return (can_exit != 0);
}

void ThreadSink::OnClientClosed(const BaseConn* conn, int task_type)
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientClosed(conn->GetConnGUID());
    }

    if (common_logic_ != NULL)
    {
        common_logic_->OnClientClosed(conn->GetConnGUID());
    }

    char client_ctx_buf[128] = "";
    const int n = StrPrintf(client_ctx_buf, sizeof(client_ctx_buf), "%s:%u, socket fd: %d",
                            conn->GetClientIP(), conn->GetClientPort(), conn->GetSockFD());

    ThreadTask* task = new ThreadTask(task_type, self_thread_, NULL, client_ctx_buf, n);
    if (NULL == task)
    {
        LOG_ERROR("failed to create tcp conn closed task");
        return;
    }

    listen_thread_->PushTask(task);
    conn_mgr_.DestroyConn(conn->GetSockFD());
}

void ThreadSink::OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len)
{
    if (common_logic_ != NULL)
    {
        common_logic_->OnRecvClientData(conn_guid, data, len);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnRecvClientData(conn_guid, data, len);
    }
}

void ThreadSink::SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups)
{
    related_thread_group_ = related_thread_groups;

    if (related_thread_group_->global_logic != NULL)
    {
        if (common_logic_ != NULL)
        {
            common_logic_->SetGlobalLogic(related_thread_group_->global_logic);
        }

        for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
        {
            LogicItem& logic_item = *it;
            logic_item.logic->SetGlobalLogic(related_thread_group_->global_logic);
        }
    }

    scheduler_.SetRelatedThreadGroups(related_thread_groups);
}

int ThreadSink::LoadCommonLogic()
{
    const std::string& tcp_common_logic_so = threads_ctx_->conf.common_logic_so;
    if (0 == tcp_common_logic_so.length())
    {
        return 0;
    }

    char common_logic_so_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(common_logic_so_path, sizeof(common_logic_so_path),
                    tcp_common_logic_so.c_str(), threads_ctx_->cur_working_dir);
    LOG_TRACE("load common logic so " << common_logic_so_path << " begin");

    if (common_logic_loader_.Load(common_logic_so_path) != 0)
    {
        LOG_ERROR("failed to load common logic so " << common_logic_so_path
                  << ", " << common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    common_logic_ = static_cast<CommonLogicInterface*>(common_logic_loader_.GetModuleInterface());
    if (NULL == common_logic_)
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
    logic_ctx.common_logic = common_logic_;
    logic_ctx.thread_ev_base = self_thread_->GetThreadEvBase();

    if (common_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_TRACE("load common logic so " << common_logic_so_path << " end");
    return 0;
}

int ThreadSink::LoadLogicGroup()
{
    // logic so group
    LogicItem logic_item;
    logic_item.logic = NULL;

    const StrGroup& logic_so_group = threads_ctx_->conf.logic_so_group;

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
        LOG_TRACE("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so " << logic_item.logic_so_path << ", "
                      << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = static_cast<LogicInterface*>(logic_item.logic_loader.GetModuleInterface());
        if (NULL == logic_item.logic)
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
        logic_ctx.scheduler = &scheduler_;
        logic_ctx.common_logic = common_logic_;
        logic_ctx.thread_ev_base = self_thread_->GetThreadEvBase();

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_TRACE("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}

int ThreadSink::OnClientConnected(const NewConnCtx* new_conn_ctx)
{
    BaseConn* conn = conn_mgr_.GetConn(new_conn_ctx->client_sock_fd);
    if (conn != NULL)
    {
        LOG_WARN("tcp conn already exist, socket fd: " << new_conn_ctx->client_sock_fd << ", destroy it first");
        conn_mgr_.DestroyConn(conn->GetSockFD());
    }

    conn = conn_mgr_.CreateConn(self_thread_->GetThreadIdx(), new_conn_ctx->client_ip,
                                new_conn_ctx->client_port, new_conn_ctx->client_sock_fd);
    if (NULL == conn)
    {
        char client_ctx_buf[128] = "";
        const int n = StrPrintf(client_ctx_buf, sizeof(client_ctx_buf), "%s:%u, socket fd: %d",
                                new_conn_ctx->client_ip, new_conn_ctx->client_port,
                                new_conn_ctx->client_sock_fd);

        ThreadTask* task = new ThreadTask(TASK_TYPE_TCP_CONN_CLOSED, self_thread_, NULL, client_ctx_buf, n);
        if (NULL == task)
        {
            LOG_ERROR("failed to create tcp conn closed task");
            return -1;
        }

        listen_thread_->PushTask(task);
        return -1;
    }

    if (common_logic_ != NULL)
    {
        common_logic_->OnClientConnected(conn->GetConnGUID());
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientConnected(conn->GetConnGUID());
    }

    return 0;
}
}
}
