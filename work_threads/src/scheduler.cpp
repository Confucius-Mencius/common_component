#include "scheduler.h"
#include "num_util.h"
#include "task_type.h"
#include "thread_sink.h"

namespace work
{
Scheduler::Scheduler()
{
    thread_sink_ = nullptr;
    msg_codec_ = nullptr;
    threads_ctx_ = nullptr;
    related_thread_groups_ = nullptr;
    last_work_thread_idx_ = 0;
    last_burden_thread_idx_ = 0;
    last_tcp_thread_idx_ = 0;
    last_proto_tcp_thread_idx_ = 0;
    last_web_thread_idx_ = 0;
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

    threads_ctx_ = static_cast<const ThreadsCtx*>(ctx);
    return 0;
}

void Scheduler::Finalize()
{
}

void Scheduler::SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups)
{
    related_thread_groups_ = related_thread_groups;

    if (related_thread_groups_->work_thread_group != nullptr)
    {
        const int work_thread_count = related_thread_groups_->work_thread_group->GetThreadCount();
        if (work_thread_count > 0)
        {
            last_work_thread_idx_ = rand() % work_thread_count;
        }
    }

    if (related_thread_groups_->burden_thread_group != nullptr)
    {
        const int burden_thread_count = related_thread_groups_->burden_thread_group->GetThreadCount();
        if (burden_thread_count > 0)
        {
            last_burden_thread_idx_ = rand() % burden_thread_count;
        }
    }

    if (related_thread_groups->tcp_thread_group != nullptr)
    {
        const int tcp_thread_count = related_thread_groups_->tcp_thread_group->GetThreadCount();
        if (tcp_thread_count > 0)
        {
            last_tcp_thread_idx_ = rand() % tcp_thread_count;
        }
    }

    if (related_thread_groups->proto_tcp_thread_group != nullptr)
    {
        const int proto_tcp_thread_count = related_thread_groups_->proto_tcp_thread_group->GetThreadCount();
        if (proto_tcp_thread_count > 0)
        {
            last_proto_tcp_thread_idx_ = rand() % proto_tcp_thread_count;
        }
    }

    if (related_thread_groups->web_thread_group != nullptr)
    {
        const int web_thread_count = related_thread_groups_->web_thread_group->GetThreadCount();
        if (web_thread_count > 0)
        {
            last_web_thread_idx_ = rand() % web_thread_count;
        }
    }
}

int Scheduler::SendToClient(const ConnGUID* conn_guid, const proto::MsgHead& msg_head,
                            const void* msg_body, size_t msg_body_len)
{
    if (nullptr == conn_guid)
    {
        return -1;
    }

    ThreadGroupInterface* thread_group = nullptr;
    ThreadInterface* thread = nullptr;

    switch (conn_guid->io_type)
    {
        case IO_TYPE_PROTO_TCP:
        {
            thread_group = related_thread_groups_->proto_tcp_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, io type: " << conn_guid->io_type);
                return -1;
            }

            thread = thread_group->GetThread(conn_guid->io_thread_idx);
        }
        break;

        default:
        {
            LOG_ERROR("invalid io type: " << conn_guid->io_type);
            return -1;
        }
        break;
    }

    if (nullptr == thread)
    {
        LOG_ERROR("no such thread, io type: " << conn_guid->io_type);
        return -1;
    }

    std::unique_ptr<char []> buf(new char[MIN_DATA_LEN + msg_body_len + 1]);
    if (nullptr == buf)
    {
        LOG_ERROR("failed to alloc memory");
        return -1;
    }

    char* data = buf.get();
    size_t len;

    if (msg_codec_->EncodeMsg(&data, len, msg_head, msg_body, msg_body_len) != 0)
    {
        return -1;
    }

    ThreadTask* task = new ThreadTask(TASK_TYPE_SEND_TO_CLIENT, thread_sink_->GetThread(), conn_guid, data, len);
    if (nullptr == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    thread->PushTask(task);
    return 0;
}

int Scheduler::SendToClient(const ConnGUID* conn_guid, const void* data, size_t len)
{
    if (nullptr == conn_guid)
    {
        return -1;
    }

    ThreadGroupInterface* thread_group = nullptr;
    ThreadInterface* thread = nullptr;

    switch (conn_guid->io_type)
    {
        case IO_TYPE_TCP:
        {
            thread_group = related_thread_groups_->tcp_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, io type: " << conn_guid->io_type);
                return -1;
            }

            thread = thread_group->GetThread(conn_guid->io_thread_idx);
        }
        break;

        case IO_TYPE_PROTO_TCP:
        {
            thread_group = related_thread_groups_->proto_tcp_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, io type: " << conn_guid->io_type);
                return -1;
            }

            thread = thread_group->GetThread(conn_guid->io_thread_idx);
        }
        break;

        case IO_TYPE_WEB_TCP:
        {
            thread_group = related_thread_groups_->web_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, io type: " << conn_guid->io_type);
                return -1;
            }

            thread = thread_group->GetThread(conn_guid->io_thread_idx);
        }
        break;

        default:
        {
            LOG_ERROR("invalid io type: " << conn_guid->io_type);
            return -1;
        }
        break;
    }

    if (nullptr == thread)
    {
        LOG_ERROR("no such thread, io type: " << conn_guid->io_type);
        return -1;
    }

    ThreadTask* task = new ThreadTask(TASK_TYPE_SEND_TO_CLIENT, thread_sink_->GetThread(), conn_guid, data, len);
    if (nullptr == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    thread->PushTask(task);
    return 0;

}

int Scheduler::CloseClient(const ConnGUID* conn_guid)
{
    if (nullptr == conn_guid)
    {
        return -1;
    }

    ThreadGroupInterface* thread_group = nullptr;
    ThreadInterface* thread = nullptr;

    switch (conn_guid->io_type)
    {
        case IO_TYPE_TCP:
        {
            thread_group = related_thread_groups_->tcp_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, io type: " << conn_guid->io_type);
                return -1;
            }

            thread = thread_group->GetThread(conn_guid->io_thread_idx);
        }
        break;

        case IO_TYPE_PROTO_TCP:
        {
            thread_group = related_thread_groups_->proto_tcp_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, io type: " << conn_guid->io_type);
                return -1;
            }

            thread = thread_group->GetThread(conn_guid->io_thread_idx);
        }
        break;

        case IO_TYPE_WEB_TCP:
        {
            thread_group = related_thread_groups_->web_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, io type: " << conn_guid->io_type);
                return -1;
            }

            thread = thread_group->GetThread(conn_guid->io_thread_idx);
        }
        break;

        default:
        {
            LOG_ERROR("invalid io type: " << conn_guid->io_type);
            return -1;
        }
        break;
    }

    if (nullptr == thread)
    {
        LOG_ERROR("no such thread, io type: " << conn_guid->io_type);
        return -1;
    }

    ThreadTask* task = new ThreadTask(TASK_TYPE_CLOSE_CONN, thread_sink_->GetThread(), conn_guid, nullptr, 0);
    if (nullptr == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    thread->PushTask(task);
    return 0;
}

int Scheduler::SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len)
{
    return SendToThread(THREAD_TYPE_GLOBAL, conn_guid, msg_head, msg_body, msg_body_len, 0);
}

int Scheduler::SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                const void* msg_body, size_t msg_body_len, int work_thread_idx)
{
    return SendToThread(THREAD_TYPE_WORK, conn_guid, msg_head, msg_body, msg_body_len, work_thread_idx);
}

int Scheduler::SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len, int burden_thread_idx)
{
    return SendToThread(THREAD_TYPE_BURDEN, conn_guid, msg_head, msg_body, msg_body_len, burden_thread_idx);
}

int Scheduler::SendToTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                               const void* msg_body, size_t msg_body_len, int tcp_thread_idx)
{
    return SendToThread(THREAD_TYPE_TCP, conn_guid, msg_head, msg_body, msg_body_len, tcp_thread_idx);
}

int Scheduler::SendToProtoTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                    const void* msg_body, size_t msg_body_len, int proto_tcp_thread_idx)
{
    return SendToThread(THREAD_TYPE_PROTO_TCP, conn_guid, msg_head, msg_body, msg_body_len, proto_tcp_thread_idx);
}

int Scheduler::SendToWebThread(const ConnGUID* conn_guid, const proto::MsgHead& msg_head,
                               const void* msg_body, size_t msg_body_len, int web_thread_idx)
{
    return SendToThread(THREAD_TYPE_WEB_TCP, conn_guid, msg_head, msg_body, msg_body_len, web_thread_idx);
}

TransID Scheduler::SendToServer(const Peer& peer, const proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len, const AsyncCtx* async_ctx)
{
    switch (peer.type)
    {
        case PEER_TYPE_PROTO_TCP:
        {
            tcp::proto::ClientInterface* tcp_client = thread_sink_->GetProtoTCPClientCenter()->GetClient(peer);
            if (nullptr == tcp_client)
            {
                tcp_client = thread_sink_->GetProtoTCPClientCenter()->CreateClient(peer);
                if (nullptr == tcp_client)
                {
                    return INVALID_TRANS_ID;
                }

                tcp_client->AddNfySink(thread_sink_);
            }

            return tcp_client->Send(msg_head, msg_body, msg_body_len, async_ctx);
        }
        break;

        default:
        {
            return INVALID_TRANS_ID;
        }
        break;
    }
}

TransID Scheduler::HTTPGet(const Peer& peer, const http::GetParams& params, const AsyncCtx* async_ctx)
{
    http::ClientInterface* http_client = thread_sink_->GetHTTPClientCenter()->GetClient(peer);
    if (nullptr == http_client)
    {
        http_client = thread_sink_->GetHTTPClientCenter()->CreateClient(peer);
        if (nullptr == http_client)
        {
            return INVALID_TRANS_ID;
        }
    }

    return http_client->Get(params, async_ctx);
}

TransID Scheduler::HTTPPost(const Peer& peer, const http::PostParams& params, const AsyncCtx* async_ctx)
{
    http::ClientInterface* http_client = thread_sink_->GetHTTPClientCenter()->GetClient(peer);
    if (nullptr == http_client)
    {
        http_client = thread_sink_->GetHTTPClientCenter()->CreateClient(peer);
        if (nullptr == http_client)
        {
            return INVALID_TRANS_ID;
        }
    }

    return http_client->Post(params, async_ctx);
}

void Scheduler::CancelTrans(TransID trans_id)
{
    thread_sink_->GetTransCenter()->CancelTrans(trans_id);
}

int Scheduler::GetScheduleWorkThreadIdx(int work_thread_idx)
{
    const int work_thread_count = related_thread_groups_->work_thread_group->GetThreadCount();

    if (INVALID_IDX(work_thread_idx, 0, work_thread_count))
    {
        work_thread_idx = last_work_thread_idx_;
        last_work_thread_idx_ = (last_work_thread_idx_ + 1) % work_thread_count;
    }

    return work_thread_idx;
}

int Scheduler::GetScheduleBurdenThreadIdx(int burden_thread_idx)
{
    const int burden_thread_count = related_thread_groups_->burden_thread_group->GetThreadCount();

    if (INVALID_IDX(burden_thread_idx, 0, burden_thread_count))
    {
        burden_thread_idx = last_burden_thread_idx_;
        last_burden_thread_idx_ = (last_burden_thread_idx_ + 1) % burden_thread_count;
    }

    return burden_thread_idx;
}

int Scheduler::GetScheduleTCPThreadIdx(int tcp_thread_idx)
{
    const int tcp_thread_count = related_thread_groups_->tcp_thread_group->GetThreadCount();

    if (INVALID_IDX(tcp_thread_idx, 0, tcp_thread_count))
    {
        tcp_thread_idx = last_tcp_thread_idx_;
        last_tcp_thread_idx_ = (last_tcp_thread_idx_ + 1) % tcp_thread_count;
    }

    return tcp_thread_idx;
}

int Scheduler::GetScheduleProtoTCPThreadIdx(int proto_tcp_thread_idx)
{
    const int proto_tcp_thread_count = related_thread_groups_->proto_tcp_thread_group->GetThreadCount();

    if (INVALID_IDX(proto_tcp_thread_idx, 0, proto_tcp_thread_count))
    {
        proto_tcp_thread_idx = last_proto_tcp_thread_idx_;
        last_proto_tcp_thread_idx_ = (last_proto_tcp_thread_idx_ + 1) % proto_tcp_thread_count;
    }

    return proto_tcp_thread_idx;
}

int Scheduler::GetScheduleWebThreadIdx(int web_thread_idx)
{
    const int web_thread_count = related_thread_groups_->web_thread_group->GetThreadCount();

    if (INVALID_IDX(web_thread_idx, 0, web_thread_count))
    {
        web_thread_idx = last_web_thread_idx_;
        last_web_thread_idx_ = (last_web_thread_idx_ + 1) % web_thread_count;
    }

    return web_thread_idx;
}

int Scheduler::SendToThread(int thread_type, const ConnGUID* conn_guid, const proto::MsgHead& msg_head,
                            const void* msg_body, size_t msg_body_len, int thread_idx)
{
    ThreadGroupInterface* thread_group = nullptr;
    ThreadInterface* thread = nullptr;
    int real_thread_idx = -1;

    switch (thread_type)
    {
        case THREAD_TYPE_GLOBAL:
        {
            thread = related_thread_groups_->global_thread;
        }
        break;

        case THREAD_TYPE_WORK:
        {
            thread_group = related_thread_groups_->work_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, thread type: " << thread_type);
                return -1;
            }

            real_thread_idx = GetScheduleWorkThreadIdx(thread_idx);
            thread = thread_group->GetThread(real_thread_idx);
        }
        break;

        case THREAD_TYPE_BURDEN:
        {
            thread_group = related_thread_groups_->burden_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, thread type: " << thread_type);
                return -1;
            }

            real_thread_idx = GetScheduleBurdenThreadIdx(thread_idx);
            thread = thread_group->GetThread(real_thread_idx);
        }
        break;

        case THREAD_TYPE_TCP:
        {
            thread_group = related_thread_groups_->tcp_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, thread type: " << thread_type);
                return -1;
            }

            real_thread_idx = GetScheduleTCPThreadIdx(thread_idx);
            thread = thread_group->GetThread(real_thread_idx);
        }
        break;

        case THREAD_TYPE_PROTO_TCP:
        {
            thread_group = related_thread_groups_->proto_tcp_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, thread type: " << thread_type);
                return -1;
            }

            real_thread_idx = GetScheduleProtoTCPThreadIdx(thread_idx);
            thread = thread_group->GetThread(real_thread_idx);
        }
        break;

        case THREAD_TYPE_WEB_TCP:
        {
            thread_group = related_thread_groups_->web_thread_group;
            if (nullptr == thread_group)
            {
                LOG_ERROR("no such threads, thread type: " << thread_type);
                return -1;
            }

            real_thread_idx = GetScheduleWebThreadIdx(thread_idx);
            thread = thread_group->GetThread(real_thread_idx);
        }
        break;

        default:
        {
            LOG_ERROR("invalid thread type: " << thread_type);
            return -1;
        }
        break;
    }

    if (nullptr == thread)
    {
        LOG_ERROR("no such thread, thread type: " << thread_type);
        return -1;
    }

    std::unique_ptr<char []> buf(new char[MIN_DATA_LEN + msg_body_len + 1]);
    if (nullptr == buf)
    {
        LOG_ERROR("failed to alloc memory");
        return -1;
    }

    char* data = buf.get();
    size_t len;

    if (msg_codec_->EncodeMsg(&data, len, msg_head, msg_body, msg_body_len) != 0)
    {
        return -1;
    }

    ThreadTask* task = new ThreadTask(TASK_TYPE_NORMAL, thread_sink_->GetThread(), conn_guid,
                                      data + TOTAL_MSG_LEN_FIELD_LEN, len - TOTAL_MSG_LEN_FIELD_LEN); // 内部的消息不发送4个字节的长度字段
    if (nullptr == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    thread->PushTask(task);
    return 0;
}
}
