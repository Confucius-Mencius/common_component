#include "burden_scheduler.h"
#include "burden_thread_sink.h"
#include "num_util.h"
#include "trans_center_interface.h"

namespace burden
{
Scheduler::Scheduler()
{
    thread_sink_ = NULL;
    related_thread_group_ = NULL;
    last_work_thread_idx_ = 0;
    last_burden_thread_idx_ = 0;
    last_tcp_thread_idx_ = 0;
    last_http_thread_idx_ = 0;
    last_udp_thread_idx_ = 0;
}

Scheduler::~Scheduler()
{
}

int Scheduler::Initialize(const void* ctx)
{
    const int burden_thread_count = thread_sink_->GetBurdenThreadGroup()->GetThreadCount();
    if (burden_thread_count > 0)
    {
        last_burden_thread_idx_ = rand() % burden_thread_count;
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
    // 如果没有work threads，那就肯定没有burden threads，所以这里不用判断有没有work threads
    const int real_work_thread_idx = GetScheduleWorkThreadIdx(work_thread_idx);
    ThreadInterface* work_thread = related_thread_group_->work_thread_group->GetThread(real_work_thread_idx);

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

int Scheduler::SendToBurdenThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                  size_t msg_body_len, int burden_thread_idx)
{
    const int real_burden_thread_idx = GetScheduleBurdenThreadIdx(burden_thread_idx);
    ThreadInterface* burden_thread = thread_sink_->GetBurdenThreadGroup()->GetThread(real_burden_thread_idx);

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

    if (burden_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::SendToGlobalThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                  size_t msg_body_len)
{
    if (NULL == related_thread_group_->global_thread)
    {
        LOG_ERROR("no global thread");
        return -1;
    }

    ThreadInterface* global_thread = related_thread_group_->global_thread;

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

int Scheduler::SendToTcpClient(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                               size_t msg_body_len)
{
    if (NULL == related_thread_group_->tcp_thread_group)
    {
        LOG_ERROR("no tcp threads");
        return -1;
    }

    ThreadInterface* tcp_thread = related_thread_group_->tcp_thread_group->GetThread(conn_guid->io_thread_idx);

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

int Scheduler::SendRawToTcpClient(const ConnGuid* conn_guid, const void* msg, size_t msg_len)
{
    if (NULL == related_thread_group_->tcp_thread_group)
    {
        LOG_ERROR("no tcp threads");
        return -1;
    }

    ThreadInterface* tcp_thread = related_thread_group_->tcp_thread_group->GetThread(conn_guid->io_thread_idx);

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

int Scheduler::CloseTcpClient(const ConnGuid* conn_guid)
{
    if (NULL == related_thread_group_->tcp_thread_group)
    {
        LOG_ERROR("no tcp threads");
        return -1;
    }

    ThreadInterface* tcp_thread = related_thread_group_->tcp_thread_group->GetThread(conn_guid->io_thread_idx);

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

int Scheduler::SendToUdpClient(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                               size_t msg_body_len)
{
    if (NULL == related_thread_group_->udp_thread_group)
    {
        LOG_ERROR("no udp threads");
        return -1;
    }

    ThreadInterface* udp_thread = related_thread_group_->udp_thread_group->GetThread(conn_guid->io_thread_idx);

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_UDP_SEND_TO_CLIENT;
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

    if (udp_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

int Scheduler::CloseUdpClient(const ConnGuid* conn_guid)
{
    if (NULL == related_thread_group_->udp_thread_group)
    {
        LOG_ERROR("no udp threads");
        return -1;
    }

    ThreadInterface* udp_thread = related_thread_group_->udp_thread_group->GetThread(conn_guid->io_thread_idx);

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_UDP_CLOSE_CONN;
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

    if (udp_thread->PushTask(task) != 0)
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

    const int real_tcp_thread_idx = GetScheduleTcpThreadIdx(tcp_thread_idx);
    ThreadInterface* tcp_thread = related_thread_group_->tcp_thread_group->GetThread(real_tcp_thread_idx);

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

int Scheduler::SendToHttpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len, int http_thread_idx)
{
    if (NULL == related_thread_group_->http_thread_group)
    {
        LOG_ERROR("no http threads");
        return -1;
    }

    const int real_http_thread_idx = GetScheduleHttpThreadIdx(http_thread_idx);
    ThreadInterface* http_thread = related_thread_group_->http_thread_group->GetThread(real_http_thread_idx);

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_BURDEN_RETURN_TO_HTTP;
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

    const int real_udp_thread_idx = GetScheduleUdpThreadIdx(udp_thread_idx);
    ThreadInterface* udp_thread = related_thread_group_->udp_thread_group->GetThread(real_udp_thread_idx);

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

    if (udp_thread->PushTask(task) != 0)
    {
        task->Release();
        return -1;
    }

    return 0;
}

void Scheduler::SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group)
{
    related_thread_group_ = related_thread_group;

    if (related_thread_group_->work_thread_group != NULL)
    {
        const int work_thread_count = related_thread_group_->work_thread_group->GetThreadCount();
        if (work_thread_count > 0)
        {
            last_work_thread_idx_ = rand() % work_thread_count;
        }
    }

    if (related_thread_group->tcp_thread_group != NULL)
    {
        const int tcp_thread_count = related_thread_group_->tcp_thread_group->GetThreadCount();
        if (tcp_thread_count > 0)
        {
            last_tcp_thread_idx_ = rand() % tcp_thread_count;
        }
    }

    if (related_thread_group_->http_thread_group != NULL)
    {
        const int http_thread_count = related_thread_group_->http_thread_group->GetThreadCount();
        if (http_thread_count > 0)
        {
            last_http_thread_idx_ = rand() % http_thread_count;
        }
    }

    if (related_thread_group_->udp_thread_group != NULL)
    {
        const int udp_thread_count = related_thread_group_->udp_thread_group->GetThreadCount();
        if (udp_thread_count > 0)
        {
            last_udp_thread_idx_ = rand() % udp_thread_count;
        }
    }
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

int Scheduler::GetScheduleBurdenThreadIdx(int burden_thread_idx)
{
    const int burden_thread_count = thread_sink_->GetBurdenThreadGroup()->GetThreadCount();

    if (INVALID_IDX(burden_thread_idx, 0, burden_thread_count))
    {
        burden_thread_idx = last_burden_thread_idx_;
        last_burden_thread_idx_ = (last_burden_thread_idx_ + 1) % burden_thread_count;
    }

    return burden_thread_idx;
}

int Scheduler::GetScheduleTcpThreadIdx(int tcp_thread_idx)
{
    const int tcp_thread_count = related_thread_group_->tcp_thread_group->GetThreadCount();

    if (INVALID_IDX(tcp_thread_idx, 0, tcp_thread_count))
    {
        tcp_thread_idx = last_tcp_thread_idx_;
        last_tcp_thread_idx_ = (last_tcp_thread_idx_ + 1) % tcp_thread_count;
    }

    return tcp_thread_idx;
}

int Scheduler::GetScheduleHttpThreadIdx(int http_thread_idx)
{
    const int http_thread_count = related_thread_group_->http_thread_group->GetThreadCount();

    if (INVALID_IDX(http_thread_idx, 0, http_thread_count))
    {
        http_thread_idx = last_http_thread_idx_;
        last_http_thread_idx_ = (last_http_thread_idx_ + 1) % http_thread_count;
    }

    return http_thread_idx;
}

int Scheduler::GetScheduleUdpThreadIdx(int udp_thread_idx)
{
    const int udp_thread_count = related_thread_group_->udp_thread_group->GetThreadCount();

    if (INVALID_IDX(udp_thread_idx, 0, udp_thread_count))
    {
        udp_thread_idx = last_udp_thread_idx_;
        last_udp_thread_idx_ = (last_udp_thread_idx_ + 1) % udp_thread_count;
    }

    return udp_thread_idx;
}
}
