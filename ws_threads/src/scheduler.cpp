#include "scheduler.h"
#include "app_frame_conf_mgr_interface.h"
#include "num_util.h"
#include "task_type.h"
#include "thread_sink.h"

namespace tcp
{
Scheduler::Scheduler()
{
    threads_ctx_ = NULL;
    thread_sink_ = NULL;
    related_thread_groups_ = NULL;
    last_tcp_thread_idx_ = 0;
    last_work_thread_idx_ = 0;
}

Scheduler::~Scheduler()
{
}

int Scheduler::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    threads_ctx_ = static_cast<const ThreadsCtx*>(ctx);

    const int tcp_thread_count = threads_ctx_->conf_mgr->GetTCPThreadCount();
    last_tcp_thread_idx_ = rand() % tcp_thread_count;

    return 0;
}

void Scheduler::Finalize()
{
}

int Scheduler::SendToClient(const ConnGUID* conn_guid, const void* data, size_t len)
{
    ThreadInterface* tcp_thread = thread_sink_->GetTCPThreadGroup()->GetThread(conn_guid->io_thread_idx);
    if (tcp_thread == thread_sink_->GetThread())
    {
        // 是自己
        BaseConn* conn = thread_sink_->GetConnMgr()->GetConnByID(conn_guid->conn_id);
        if (NULL == conn)
        {
            LOG_ERROR("failed to get tcp conn by id: " << conn_guid->conn_id);
            return -1;
        }

        return conn->Send(data, len);
    }

    // 是其它的tcp线程
    ThreadTask* task = new ThreadTask(TASK_TYPE_TCP_SEND_TO_CLIENT, thread_sink_->GetThread(), conn_guid, data, len);
    if (NULL == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    tcp_thread->PushTask(task);
    return 0;
}

int Scheduler::CloseClient(const ConnGUID* conn_guid)
{
    ThreadInterface* tcp_thread = thread_sink_->GetTCPThreadGroup()->GetThread(conn_guid->io_thread_idx);
    if (tcp_thread == thread_sink_->GetThread())
    {
        BaseConn* conn = thread_sink_->GetConnMgr()->GetConnByID(conn_guid->conn_id);
        if (NULL == conn)
        {
            LOG_ERROR("failed to get tcp conn by id: " << conn_guid->conn_id);
            return -1;
        }

        thread_sink_->OnClientClosed(conn);
        return 0;
    }

    ThreadTask* task = new ThreadTask(TASK_TYPE_TCP_CLOSE_CONN, thread_sink_->GetThread(), conn_guid, NULL, 0);
    if (NULL == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    tcp_thread->PushTask(task);
    return 0;
}

int Scheduler::SendToTCPThread(const ConnGUID* conn_guid, const void* data, size_t len, int tcp_thread_idx)
{
    const int real_tcp_thread_idx = GetScheduleTCPThreadIdx(tcp_thread_idx);
    ThreadInterface* tcp_thread = thread_sink_->GetTCPThreadGroup()->GetThread(real_tcp_thread_idx);

    ThreadTask* task = new ThreadTask(TASK_TYPE_NORMAL, thread_sink_->GetThread(), conn_guid, data, len);
    if (NULL == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    tcp_thread->PushTask(task);
    return 0;
}

int Scheduler::SendToWorkThread(const ConnGUID* conn_guid, const void* data, size_t len, int work_thread_idx)
{
    if (NULL == related_thread_groups_->work_threads)
    {
        LOG_ERROR("no work threads");
        return -1;
    }

    const int real_work_thread_idx = GetScheduleWorkThreadIdx(work_thread_idx);
    ThreadInterface* work_thread = related_thread_groups_->work_threads->GetThread(real_work_thread_idx);

    ThreadTask* task = new ThreadTask(TASK_TYPE_NORMAL, thread_sink_->GetThread(), conn_guid, data, len);
    if (NULL == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    work_thread->PushTask(task);
    return 0;
}

int Scheduler::SendToGlobalThread(const ConnGUID* conn_guid, const void* data, size_t len)
{
    if (NULL == related_thread_groups_->global_thread)
    {
        LOG_ERROR("no global thread");
        return -1;
    }

    ThreadInterface* global_thread = related_thread_groups_->global_thread;

    ThreadTask* task = new ThreadTask(TASK_TYPE_NORMAL, thread_sink_->GetThread(), conn_guid, data, len);
    if (NULL == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    global_thread->PushTask(task);
    return 0;
}

void Scheduler::SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups)
{
    related_thread_groups_ = related_thread_groups;

    if (related_thread_groups_->work_threads != NULL)
    {
        const int work_thread_count = related_thread_groups_->work_threads->GetThreadCount();
        if (work_thread_count > 0)
        {
            last_work_thread_idx_ = rand() % work_thread_count;
        }
    }
}

int Scheduler::GetScheduleTCPThreadIdx(int tcp_thread_idx)
{
    const int tcp_thread_count = thread_sink_->GetTCPThreadGroup()->GetThreadCount();

    if (INVALID_IDX(tcp_thread_idx, 0, tcp_thread_count))
    {
        tcp_thread_idx = last_tcp_thread_idx_;
        last_tcp_thread_idx_ = (last_tcp_thread_idx_ + 1) % tcp_thread_count;
    }

    return tcp_thread_idx;
}

int Scheduler::GetScheduleWorkThreadIdx(int work_thread_idx)
{
    const int work_thread_count = related_thread_groups_->work_threads->GetThreadCount();

    if (INVALID_IDX(work_thread_idx, 0, work_thread_count))
    {
        work_thread_idx = last_work_thread_idx_;
        last_work_thread_idx_ = (last_work_thread_idx_ + 1) % work_thread_count;
    }

    return work_thread_idx;
}
}
