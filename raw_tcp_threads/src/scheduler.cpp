#include "scheduler.h"
#include "app_frame_conf_mgr_interface.h"
#include "num_util.h"
#include "task_type.h"
#include "thread_sink.h"

namespace tcp
{
namespace raw
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

    const int tcp_thread_count = threads_ctx_->conf.thread_count;
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
        BaseConn* conn = static_cast<BaseConn*>(thread_sink_->GetConnMgr()->GetConnByID(conn_guid->conn_id));
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
        BaseConn* conn = static_cast<BaseConn*>(thread_sink_->GetConnMgr()->GetConnByID(conn_guid->conn_id));
        if (NULL == conn)
        {
            LOG_ERROR("failed to get tcp conn by id: " << conn_guid->conn_id);
            return -1;
        }

        thread_sink_->OnClientClosed(conn, TASK_TYPE_TCP_CONN_CLOSED);
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
    return SendToThread(THREAD_TYPE_TCP, conn_guid, data, len, tcp_thread_idx);
}

int Scheduler::SendToWorkThread(const ConnGUID* conn_guid, const void* data, size_t len, int work_thread_idx)
{
    return SendToThread(THREAD_TYPE_WORK, conn_guid, data, len, work_thread_idx);
}

int Scheduler::SendToGlobalThread(const ConnGUID* conn_guid, const void* data, size_t len)
{
    return SendToThread(THREAD_TYPE_GLOBAL, conn_guid, data, len, 0);
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

int Scheduler::SendToThread(int thread_type, const ConnGUID* conn_guid, const void* data, size_t len, int thread_idx)
{
    ThreadGroupInterface* thread_group = NULL;
    ThreadInterface* thread = NULL;
    int real_thread_idx = -1;

    switch (thread_type)
    {
        case THREAD_TYPE_TCP:
        {
            thread_group = thread_sink_->GetTCPThreadGroup();
            if (NULL == thread_group)
            {
                LOG_ERROR("no such threads, thread type: " << thread_type);
                return -1;
            }

            real_thread_idx = GetScheduleTCPThreadIdx(thread_idx);
            thread = thread_group->GetThread(real_thread_idx);

        }
        break;

        case THREAD_TYPE_WORK:
        {
            thread_group = related_thread_groups_->work_threads;
            if (NULL == thread_group)
            {
                LOG_ERROR("no such threads, thread type: " << thread_type);
                return -1;
            }

            real_thread_idx = GetScheduleWorkThreadIdx(thread_idx);
            thread = thread_group->GetThread(real_thread_idx);
        }
        break;

        case THREAD_TYPE_GLOBAL:
        {
            thread = related_thread_groups_->global_thread;
        }
        break;

        default:
        {
        }
        break;
    }

    if (NULL == thread)
    {
        LOG_ERROR("no such thread, thread type: " << thread_type);
        return -1;
    }

    ThreadTask* task = new ThreadTask(TASK_TYPE_NORMAL, thread, conn_guid, data, len);
    if (NULL == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    thread->PushTask(task);
    return 0;
}
}
}
