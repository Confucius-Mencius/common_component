#include "tcp_scheduler.h"
#include "app_frame_conf_mgr_interface.h"
#include "num_util.h"
#include "tcp_thread_sink.h"

namespace tcp
{
Scheduler::Scheduler()
{
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

    threads_ctx_ = static_cast<const ThreadsCtx* threads_ctx_>(ctx);

    const int tcp_thread_count = threads_ctx_->conf_mgr->GetTCPThreadCount();
    if (tcp_thread_count > 0)
    {
        last_tcp_thread_idx_ = rand() % tcp_thread_count;
    }

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
        ConnInterface* conn = thread_sink_->GetConnMgr()->GetConnByConnId(conn_guid->conn_id);
        if (NULL == conn)
        {
            LOG_ERROR("failed to get tcp conn by id: " << conn_guid->conn_id);
            return -1;
        }

        return conn->Send(msg_head, msg_body, msg_body_len, -1);
    }

    // 是其它的tcp线程
    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_TCP_SEND_TO_CLIENT;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != NULL)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    task_ctx.msg_head = msg_head;
    task_ctx.msg_body = (char*) msg_body;
    task_ctx.msg_body_len = msg_body_len;

    Task* task = Task::Create(&task_ctx);
    if (NULL == task)
    {
        return -1;
    }

    if (tcp_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::SendRawToClient(const ConnGUID* conn_guid, const void* msg, size_t msg_len)
{
    ThreadInterface* tcp_thread = thread_sink_->GetTcpThreadGroup()->GetThread(conn_guid->io_thread_idx);
    if (tcp_thread == thread_sink_->GetThread())
    {
        // 是自己
        ConnInterface* conn = thread_sink_->GetConnCenter()->GetConnByConnId(conn_guid->conn_id);
        if (NULL == conn)
        {
            LOG_ERROR("failed to get tcp conn by id: " << conn_guid->conn_id);
            return -1;
        }

        return conn->Send(msg, msg_len, -1);
    }

    // 是其它的tcp线程
    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_TCP_SEND_RAW_TO_CLIENT;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != NULL)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    task_ctx.msg_body = (char*) msg;
    task_ctx.msg_body_len = msg_len;

    Task* task = Task::Create(&task_ctx);
    if (NULL == task)
    {
        return -1;
    }

    if (tcp_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::CloseClient(const ConnGUID* conn_guid)
{
    ThreadInterface* tcp_thread = thread_sink_->GetTcpThreadGroup()->GetThread(conn_guid->io_thread_idx);
    if (tcp_thread == thread_sink_->GetThread())
    {
        ConnInterface* conn = thread_sink_->GetConnCenter()->GetConnByConnId(conn_guid->conn_id);
        if (NULL == conn)
        {
            LOG_ERROR("failed to get tcp conn by id: " << conn_guid->conn_id);
            return -1;
        }

        thread_sink_->OnClientClosed(conn);
        return 0;
    }

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_TCP_CLOSE_CONN;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != NULL)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    Task* task = Task::Create(&task_ctx);
    if (NULL == task)
    {
        return -1;
    }

    if (tcp_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::SendToTCPThread(const ConnGUID* conn_guid, const MsgHead& msg_head, const void* msg_body,
                               size_t msg_body_len, int tcp_thread_idx)
{
    const int real_tcp_thread_idx = GetScheduleTCPThreadIdx(tcp_thread_idx);
    ThreadInterface* tcp_thread = thread_sink_->GetTcpThreadGroup()->GetThread(real_tcp_thread_idx);

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_NORMAL;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != NULL)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    task_ctx.msg_head = msg_head;
    task_ctx.msg_body = (char*) msg_body;
    task_ctx.msg_body_len = msg_body_len;

    Task* task = Task::Create(&task_ctx);
    if (NULL == task)
    {
        return -1;
    }

    if (tcp_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::SendToWorkThread(const ConnGUID* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len, int work_thread_idx)
{
    if (NULL == related_thread_groups_->work_threads)
    {
        LOG_ERROR("no work threads");
        return -1;
    }

    const int real_work_thread_idx = GetScheduleWorkThreadIdx(work_thread_idx);
    ThreadInterface* work_thread = related_thread_groups_->work_threads->GetThread(real_work_thread_idx);

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_NORMAL;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != NULL)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    task_ctx.msg_head = msg_head;
    task_ctx.msg_body = (char*) msg_body;
    task_ctx.msg_body_len = msg_body_len;

    Task* task = Task::Create(&task_ctx);
    if (NULL == task)
    {
        return -1;
    }

    if (work_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::SendToGlobalThread(const ConnGUID* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                  size_t msg_body_len)
{
    if (NULL == related_thread_groups_->global_thread)
    {
        LOG_ERROR("no global thread");
        return -1;
    }

    ThreadInterface* global_thread = related_thread_groups_->global_thread;

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_NORMAL;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != NULL)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    task_ctx.msg_head = msg_head;
    task_ctx.msg_body = (char*) msg_body;
    task_ctx.msg_body_len = msg_body_len;

    Task* task = Task::Create(&task_ctx);
    if (NULL == task)
    {
        return -1;
    }

    if (global_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

void Scheduler::SetRelatedThreadGroup(RelatedThreadGroups* related_thread_group)
{
    related_thread_groups_ = related_thread_group;

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
    const int tcp_thread_count = thread_sink_->GetTcpThreadGroup()->GetThreadCount();

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
