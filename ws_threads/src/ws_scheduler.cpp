#include "ws_scheduler.h"
#include "app_frame_conf_mgr_interface.h"
#include "num_util.h"
#include "task_type.h"
#include "thread_sink.h"

namespace ws
{
Scheduler::Scheduler()
{
    threads_ctx_ = NULL;
    thread_sink_ = NULL;
    related_thread_groups_ = NULL;
    last_ws_thread_idx_ = 0;
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

    const int ws_thread_count = threads_ctx_->conf_mgr->GetWSThreadCount();
    last_ws_thread_idx_ = rand() % ws_thread_count;

    return 0;
}

void Scheduler::Finalize()
{
}

int Scheduler::SendToClient(const ConnGUID* conn_guid, const void* data, size_t len)
{
    ThreadInterface* ws_thread = thread_sink_->GetWSThreadGroup()->GetThread(conn_guid->io_thread_idx);
    if (ws_thread == thread_sink_->GetThread())
    {
        // 是自己
        BaseConn* conn = thread_sink_->GetConnMgr()->GetConnByID(conn_guid->conn_id);
        if (NULL == conn)
        {
            LOG_ERROR("failed to get ws conn by id: " << conn_guid->conn_id);
            return -1;
        }

        return conn->Send(data, len);
    }

    // 是其它的ws线程
    ThreadTask* task = new ThreadTask(TASK_TYPE_WS_SEND_TO_CLIENT, thread_sink_->GetThread(), conn_guid, data, len);
    if (NULL == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    ws_thread->PushTask(task);
    return 0;
}

int Scheduler::CloseClient(const ConnGUID* conn_guid)
{
    ThreadInterface* ws_thread = thread_sink_->GetWSThreadGroup()->GetThread(conn_guid->io_thread_idx);
    if (ws_thread == thread_sink_->GetThread())
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

    ThreadTask* task = new ThreadTask(TASK_TYPE_WS_CLOSE_CONN, thread_sink_->GetThread(), conn_guid, NULL, 0);
    if (NULL == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    ws_thread->PushTask(task);
    return 0;
}

int Scheduler::SendToWSThread(const ConnGUID* conn_guid, const void* data, size_t len, int ws_thread_idx)
{
    const int real_ws_thread_idx = GetScheduleWSThreadIdx(ws_thread_idx);
    ThreadInterface* ws_thread = thread_sink_->GetWSThreadGroup()->GetThread(real_ws_thread_idx);

    ThreadTask* task = new ThreadTask(TASK_TYPE_NORMAL, thread_sink_->GetThread(), conn_guid, data, len);
    if (NULL == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    ws_thread->PushTask(task);
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

int Scheduler::GetScheduleWSThreadIdx(int ws_thread_idx)
{
    const int ws_thread_count = thread_sink_->GetWSThreadGroup()->GetThreadCount();

    if (INVALID_IDX(ws_thread_idx, 0, ws_thread_count))
    {
        ws_thread_idx = last_ws_thread_idx_;
        last_ws_thread_idx_ = (last_ws_thread_idx_ + 1) % ws_thread_count;
    }

    return ws_thread_idx;
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
