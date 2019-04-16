#include "scheduler.h"
#include "app_frame_conf_mgr_interface.h"
#include "num_util.h"
#include "trans_center_interface.h"
#include "thread_sink.h"

namespace udp
{
Scheduler::Scheduler()
{
    thread_sink_ = nullptr;
    related_thread_group_ = nullptr;
    last_udp_thread_idx_ = 0;
    last_work_thread_idx_ = 0;
}

Scheduler::~Scheduler()
{
}

int Scheduler::Initialize(const void* ctx)
{
    if (nullptr == ctx)
    {
        return -1;
    }

    threads_ctx_ = (ThreadsCtx*) ctx;

    const int udp_thread_count = threads_ctx_->conf_mgr->GetUdpThreadCount();
    if (udp_thread_count > 0)
    {
        last_udp_thread_idx_ = rand() % udp_thread_count;
    }

    return 0;
}

void Scheduler::Finalize()
{
}

struct event_base* Scheduler::GetThreadEvBase() const
{
    return thread_sink_->GetThread()->GetThreadEvBase();
}

int Scheduler::GetThreadIdx() const
{
    return thread_sink_->GetThread()->GetThreadIdx();
}

TransId Scheduler::SendToServer(const Peer& peer, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len,
                                const base::AsyncCtx* async_ctx)
{
    switch (peer.type)
    {
        case PEER_TYPE_TCP:
        {
            tcp::ClientInterface* tcp_client = thread_sink_->GetTcpClientCenter()->GetClient(peer);
            if (nullptr == tcp_client)
            {
                tcp_client = thread_sink_->GetTcpClientCenter()->CreateClient(peer);
                if (nullptr == tcp_client)
                {
                    return INVALID_TRANS_ID;
                }

                tcp_client->AddNfySink(thread_sink_);
            }

            return tcp_client->Send(msg_head, msg_body, msg_body_len, async_ctx);
        }
        break;

        case PEER_TYPE_UDP:
        {
            udp::ClientInterface* udp_client = thread_sink_->GetUdpClientCenter()->GetClient(peer);
            if (nullptr == udp_client)
            {
                udp_client = thread_sink_->GetUdpClientCenter()->CreateClient(peer);
                if (nullptr == udp_client)
                {
                    return INVALID_TRANS_ID;
                }
            }

            return udp_client->Send(msg_head, msg_body, msg_body_len, async_ctx);
        }
        break;

        default:
        {
            return INVALID_TRANS_ID;
        }
        break;
    }
}

TransId Scheduler::HttpGet(const Peer& peer, const http::GetParams& params, const base::AsyncCtx* async_ctx)
{
    http::ClientInterface* http_client = thread_sink_->GetHttpClientCenter()->GetClient(peer);
    if (nullptr == http_client)
    {
        http_client = thread_sink_->GetHttpClientCenter()->CreateClient(peer);
        if (nullptr == http_client)
        {
            return INVALID_TRANS_ID;
        }
    }

    return http_client->Get(params, async_ctx);
}

TransId Scheduler::HttpPost(const Peer& peer, const http::PostParams& params, const base::AsyncCtx* async_ctx)
{
    http::ClientInterface* http_client = thread_sink_->GetHttpClientCenter()->GetClient(peer);
    if (nullptr == http_client)
    {
        http_client = thread_sink_->GetHttpClientCenter()->CreateClient(peer);
        if (nullptr == http_client)
        {
            return INVALID_TRANS_ID;
        }
    }

    return http_client->Post(params, async_ctx);
}

TransId Scheduler::HttpHead(const Peer& peer, const http::HeadParams& params, const base::AsyncCtx* async_ctx)
{
    http::ClientInterface* http_client = thread_sink_->GetHttpClientCenter()->GetClient(peer);
    if (nullptr == http_client)
    {
        http_client = thread_sink_->GetHttpClientCenter()->CreateClient(peer);
        if (nullptr == http_client)
        {
            return INVALID_TRANS_ID;
        }
    }

    return http_client->Head(params, async_ctx);
}

void Scheduler::CancelTrans(TransId trans_id)
{
    // 一个线程里面只有一个trans center
    thread_sink_->GetThread()->GetTransCenter()->CancelTrans(trans_id);
}

int Scheduler::SendToClient(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                            size_t msg_body_len)
{
    ThreadInterface* udp_thread = thread_sink_->GetUdpThreadGroup()->GetThread(conn_guid->io_thread_idx);
    if (udp_thread == thread_sink_->GetThread())
    {
        // 是自己
        ConnInterface* conn = thread_sink_->GetConnCenter()->GetConnByConnId(conn_guid->conn_id);
        if (nullptr == conn)
        {
            LOG_ERROR("failed to get udp conn by id: " ->conn_id);
            return -1;
        }

        return conn->Send(msg_head, msg_body, msg_body_len, -1);
    }

    // 是其它的udp线程
    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_UDP_SEND_TO_CLIENT;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != nullptr)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    task_ctx.msg_head = msg_head;
    task_ctx.msg_body = (char*) msg_body;
    task_ctx.msg_body_len = msg_body_len;

    Task* task = Task::Create(&task_ctx);
    if (nullptr == task)
    {
        return -1;
    }

    if (udp_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::CloseClient(const ConnGuid* conn_guid)
{
    ThreadInterface* udp_thread = thread_sink_->GetUdpThreadGroup()->GetThread(conn_guid->io_thread_idx);
    if (udp_thread == thread_sink_->GetThread())
    {
        ConnInterface* conn = thread_sink_->GetConnCenter()->GetConnByConnId(conn_guid->conn_id);
        if (nullptr == conn)
        {
            LOG_ERROR("failed to get udp conn by id: " ->conn_id);
            return -1;
        }

        thread_sink_->GetConnCenter()->RemoveConn(conn->GetClientIp(), conn->GetClientPort());
        return 0;
    }

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_UDP_CLOSE_CONN;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != nullptr)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    Task* task = Task::Create(&task_ctx);
    if (nullptr == task)
    {
        return -1;
    }

    if (udp_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::SendToUdpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                               size_t msg_body_len, int udp_thread_idx)
{
    const int real_udp_thread_idx = GetScheduleUdpThreadIdx(udp_thread_idx);
    ThreadInterface* udp_thread = thread_sink_->GetUdpThreadGroup()->GetThread(real_udp_thread_idx);

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_NORMAL;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != nullptr)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    task_ctx.msg_head = msg_head;
    task_ctx.msg_body = (char*) msg_body;
    task_ctx.msg_body_len = msg_body_len;

    Task* task = Task::Create(&task_ctx);
    if (nullptr == task)
    {
        return -1;
    }

    if (udp_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::SendToWorkThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len, int work_thread_idx)
{
    if (nullptr == related_thread_group_->work_thread_group)
    {
        LOG_ERROR("no work threads");
        return -1;
    }

    const int real_work_thread_idx = GetScheduleWorkThreadIdx(work_thread_idx);
    ThreadInterface* work_thread = related_thread_group_->work_thread_group->GetThread(real_work_thread_idx);

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_NORMAL;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != nullptr)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    task_ctx.msg_head = msg_head;
    task_ctx.msg_body = (char*) msg_body;
    task_ctx.msg_body_len = msg_body_len;

    Task* task = Task::Create(&task_ctx);
    if (nullptr == task)
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

int Scheduler::SendToGlobalThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                  size_t msg_body_len)
{
    if (nullptr == related_thread_group_->global_thread)
    {
        LOG_ERROR("no global thread");
        return -1;
    }

    ThreadInterface* global_thread = related_thread_group_->global_thread;

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_NORMAL;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != nullptr)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    task_ctx.msg_head = msg_head;
    task_ctx.msg_body = (char*) msg_body;
    task_ctx.msg_body_len = msg_body_len;

    Task* task = Task::Create(&task_ctx);
    if (nullptr == task)
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
    related_thread_group_ = related_thread_group;

    if (related_thread_group_->work_thread_group != nullptr)
    {
        const int work_thread_count = related_thread_group_->work_thread_group->GetThreadCount();
        if (work_thread_count > 0)
        {
            last_work_thread_idx_ = rand() % work_thread_count;
        }
    }
}

int Scheduler::GetScheduleUdpThreadIdx(int udp_thread_idx)
{
    const int udp_thread_count = thread_sink_->GetUdpThreadGroup()->GetThreadCount();

    if (INVALID_IDX(udp_thread_idx, 0, udp_thread_count))
    {
        udp_thread_idx = last_udp_thread_idx_;
        last_udp_thread_idx_ = (last_udp_thread_idx_ + 1) % udp_thread_count;
    }

    return udp_thread_idx;
}

int Scheduler::GetScheduleWorkThreadIdx(int work_thread_idx)
{
    const int work_thread_count = related_thread_group_->work_thread_group->GetThreadCount();

    if (INVALID_IDX(work_thread_idx, 0, work_thread_count))
    {
        work_thread_idx = last_work_thread_idx_;
        last_work_thread_idx_ = (last_work_thread_idx_ + 1) % work_thread_count;
    }

    return work_thread_idx;
}
}
