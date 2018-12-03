#include "http_scheduler.h"
#include "http_thread_sink.h"
#include "num_util.h"
#include "trans_center_interface.h"

namespace http
{
Scheduler::Scheduler()
{
    thread_sink_ = NULL;
    related_thread_group_ = NULL;
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

    threads_ctx_ = (const ThreadsCtx*) ctx;
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

int Scheduler::SendToClient(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, int http_code,
                            const KeyValMap* http_header_map, const char* content, size_t content_len)
{
    ThreadInterface* http_thread = thread_sink_->GetHttpThreadGroup()->GetThread(conn_guid->io_thread_idx);
    if (http_thread == thread_sink_->GetThread())
    {
        // 是自己
        ConnInterface* conn = thread_sink_->GetConnCenter()->GetConnByConnId(conn_guid->conn_id);
        if (NULL == conn)
        {
            LOG_ERROR("failed to get http conn by id: " << conn_guid->conn_id);
            return -1;
        }

        return conn->Send(evhttp_req, http_code, http_header_map, content, content_len);
    }

    // 是其它的http线程
    LOG_ERROR("can not send to other http thread manager'd conn");
    return -1;
}

TransId Scheduler::SendToWorkThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                    size_t msg_body_len, int work_thread_idx, const base::AsyncCtx* async_ctx)
{
    if (NULL == related_thread_group_->work_thread_group)
    {
        LOG_ERROR("no work threads");
        return INVALID_TRANS_ID;
    }

    TransCtx trans_ctx;
    trans_ctx.peer.type = PEER_TYPE_THREAD;
    trans_ctx.passback = msg_head.passback;

    if (async_ctx != NULL)
    {
        trans_ctx.timeout_sec = async_ctx->timeout_sec;
        trans_ctx.sink = async_ctx->sink;
        trans_ctx.async_data = (char*) async_ctx->async_data;
        trans_ctx.async_data_len = async_ctx->async_data_len;
    }

    TransId trans_id = thread_sink_->GetThread()->GetTransCenter()->RecordTransCtx(&trans_ctx);
    if (INVALID_TRANS_ID == trans_id)
    {
        return trans_id;
    }

    MsgHead trans_msg_head = msg_head;
    trans_msg_head.passback = trans_id;

    const int real_work_thread_idx = GetScheduleWorkThreadIdx(work_thread_idx);
    ThreadInterface* work_thread = related_thread_group_->work_thread_group->GetThread(real_work_thread_idx);

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_NORMAL;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != NULL)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    task_ctx.msg_head = trans_msg_head;
    task_ctx.msg_body = (char*) msg_body;
    task_ctx.msg_body_len = msg_body_len;

    Task* task = Task::Create(&task_ctx);
    if (NULL == task)
    {
        thread_sink_->GetThread()->GetTransCenter()->CancelTrans(trans_id);
        return INVALID_TRANS_ID;
    }

    if (work_thread->PushTask(task) != 0)
    {
        task->Release();
        thread_sink_->GetThread()->GetTransCenter()->CancelTrans(trans_id);
        return INVALID_TRANS_ID;
    }

    return trans_id;
}

TransId Scheduler::SendToGlobalThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                      size_t msg_body_len, const base::AsyncCtx* async_ctx)
{
    if (NULL == related_thread_group_->global_thread)
    {
        LOG_ERROR("no global thread");
        return INVALID_TRANS_ID;
    }

    TransCtx trans_ctx;
    trans_ctx.peer.type = PEER_TYPE_THREAD;
    trans_ctx.passback = msg_head.passback;

    if (async_ctx != NULL)
    {
        trans_ctx.timeout_sec = async_ctx->timeout_sec;
        trans_ctx.sink = async_ctx->sink;
        trans_ctx.async_data = (char*) async_ctx->async_data;
        trans_ctx.async_data_len = async_ctx->async_data_len;
    }

    TransId trans_id = thread_sink_->GetThread()->GetTransCenter()->RecordTransCtx(&trans_ctx);
    if (INVALID_TRANS_ID == trans_id)
    {
        return trans_id;
    }

    MsgHead trans_msg_head = msg_head;
    trans_msg_head.passback = trans_id;

    ThreadInterface* global_thread = related_thread_group_->global_thread;

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_NORMAL;
    task_ctx.source_thread = thread_sink_->GetThread();

    if (conn_guid != NULL)
    {
        task_ctx.conn_guid = *conn_guid;
    }

    task_ctx.msg_head = trans_msg_head;
    task_ctx.msg_body = (char*) msg_body;
    task_ctx.msg_body_len = msg_body_len;

    Task* task = Task::Create(&task_ctx);
    if (NULL == task)
    {
        thread_sink_->GetThread()->GetTransCenter()->CancelTrans(trans_id);
        return INVALID_TRANS_ID;
    }

    if (global_thread->PushTask(task) != 0)
    {
        task->Release();
        thread_sink_->GetThread()->GetTransCenter()->CancelTrans(trans_id);
        return INVALID_TRANS_ID;
    }

    return trans_id;
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
