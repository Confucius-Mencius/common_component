#include "global_scheduler.h"
#include "global_thread_sink.h"
#include "num_util.h"
#include "trans_center_interface.h"

namespace global
{
Scheduler::Scheduler()
{
    thread_sink_ = NULL;
    related_thread_group_ = NULL;
}

Scheduler::~Scheduler()
{
}

int Scheduler::Initialize(const void* ctx)
{
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
            if (NULL == tcp_client)
            {
                tcp_client = thread_sink_->GetTcpClientCenter()->CreateClient(peer);
                if (NULL == tcp_client)
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
            if (NULL == udp_client)
            {
                udp_client = thread_sink_->GetUdpClientCenter()->CreateClient(peer);
                if (NULL == udp_client)
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
    if (NULL == http_client)
    {
        http_client = thread_sink_->GetHttpClientCenter()->CreateClient(peer);
        if (NULL == http_client)
        {
            return INVALID_TRANS_ID;
        }
    }

    return http_client->Get(params, async_ctx);
}

TransId Scheduler::HttpPost(const Peer& peer, const http::PostParams& params, const base::AsyncCtx* async_ctx)
{
    http::ClientInterface* http_client = thread_sink_->GetHttpClientCenter()->GetClient(peer);
    if (NULL == http_client)
    {
        http_client = thread_sink_->GetHttpClientCenter()->CreateClient(peer);
        if (NULL == http_client)
        {
            return INVALID_TRANS_ID;
        }
    }

    return http_client->Post(params, async_ctx);
}

TransId Scheduler::HttpHead(const Peer& peer, const http::HeadParams& params, const base::AsyncCtx* async_ctx)
{
    http::ClientInterface* http_client = thread_sink_->GetHttpClientCenter()->GetClient(peer);
    if (NULL == http_client)
    {
        http_client = thread_sink_->GetHttpClientCenter()->CreateClient(peer);
        if (NULL == http_client)
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

int Scheduler::SendToWorkThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len, int work_thread_idx)
{
    if (NULL == related_thread_group_->work_thread_group)
    {
        LOG_ERROR("no work threads");
        return -1;
    }

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

    const int work_thread_count = related_thread_group_->work_thread_group->GetThreadCount();

    if (-1 == work_thread_idx)
    {
        for (int i = 0; i < work_thread_count; ++i)
        {
            ThreadInterface* work_thread = related_thread_group_->work_thread_group->GetThread(i);
            work_thread->PushTask(task->Clone());
        }

        task->Release();
        return 0;
    }

    if (INVALID_IDX(work_thread_idx, 0, work_thread_count))
    {
        task->Release();
        return -1;
    }

    ThreadInterface* work_thread = related_thread_group_->work_thread_group->GetThread(work_thread_idx);

    if (work_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::SendToBurdenThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                  size_t msg_body_len, int burden_thread_idx)
{
    if (NULL == related_thread_group_->burden_thread_group)
    {
        LOG_ERROR("no burden threads");
        return -1;
    }

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

    const int burden_thread_count = related_thread_group_->burden_thread_group->GetThreadCount();

    if (-1 == burden_thread_idx)
    {
        for (int i = 0; i < burden_thread_count; ++i)
        {
            ThreadInterface* burden_thread = related_thread_group_->burden_thread_group->GetThread(i);
            burden_thread->PushTask(task->Clone());
        }

        task->Release();
        return 0;
    }

    if (INVALID_IDX(burden_thread_idx, 0, burden_thread_count))
    {
        task->Release();
        return -1;
    }

    ThreadInterface* burden_thread = related_thread_group_->burden_thread_group->GetThread(burden_thread_idx);

    if (burden_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::SendToTcpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                               size_t msg_body_len, int tcp_thread_idx)
{
    if (NULL == related_thread_group_->tcp_thread_group)
    {
        LOG_ERROR("no tcp threads");
        return -1;
    }

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

    const int tcp_thread_count = related_thread_group_->tcp_thread_group->GetThreadCount();

    if (-1 == tcp_thread_idx)
    {
        for (int i = 0; i < tcp_thread_count; ++i)
        {
            ThreadInterface* tcp_thread = related_thread_group_->tcp_thread_group->GetThread(i);
            tcp_thread->PushTask(task->Clone());
        }

        task->Release();
        return 0;
    }

    if (INVALID_IDX(tcp_thread_idx, 0, tcp_thread_count))
    {
        task->Release();
        return -1;
    }

    ThreadInterface* tcp_thread = related_thread_group_->tcp_thread_group->GetThread(tcp_thread_idx);

    if (tcp_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::SendToHttpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len, int http_thread_idx)
{
    if (NULL == related_thread_group_->http_thread_group)
    {
        LOG_ERROR("no http threads");
        return -1;
    }

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_GLOBAL_RETURN_TO_HTTP;
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

    const int http_thread_count = related_thread_group_->http_thread_group->GetThreadCount();

    if (INVALID_IDX(http_thread_idx, 0, http_thread_count))
    {
        task->Release();
        return -1;
    }

    ThreadInterface* http_thread = related_thread_group_->http_thread_group->GetThread(http_thread_idx);

    if (http_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::SendToUdpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                               size_t msg_body_len, int udp_thread_idx)
{
    if (NULL == related_thread_group_->udp_thread_group)
    {
        LOG_ERROR("no udp threads");
        return -1;
    }

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

    const int udp_thread_count = related_thread_group_->udp_thread_group->GetThreadCount();

    if (-1 == udp_thread_idx)
    {
        for (int i = 0; i < udp_thread_count; ++i)
        {
            ThreadInterface* udp_thread = related_thread_group_->udp_thread_group->GetThread(i);
            udp_thread->PushTask(task->Clone());
        }

        task->Release();
        return 0;
    }

    if (INVALID_IDX(udp_thread_idx, 0, udp_thread_count))
    {
        task->Release();
        return -1;
    }

    ThreadInterface* udp_thread = related_thread_group_->udp_thread_group->GetThread(udp_thread_idx);

    if (udp_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}
}
