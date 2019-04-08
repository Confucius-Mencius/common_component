#include "scheduler.h"
#include "thread_sink.h"
#include "num_util.h"
#include "task_type.h"

namespace global
{
Scheduler::Scheduler()
{
    thread_sink_ = nullptr;
    msg_codec_ = nullptr;
    threads_ctx_ = nullptr;
    related_thread_groups_ = nullptr;
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

int Scheduler::SendToRawTCPThread(const ConnGUID* conn_guid, const proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len, int raw_tcp_thread_idx)
{
    return SendToThread(THREAD_TYPE_RAW_TCP, conn_guid, msg_head, msg_body, msg_body_len, raw_tcp_thread_idx);
}

int Scheduler::SendToProtoTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                    const void* msg_body, size_t msg_body_len, int proto_tcp_thread_idx)
{
    return SendToThread(THREAD_TYPE_PROTO_TCP, conn_guid, msg_head, msg_body, msg_body_len, proto_tcp_thread_idx);
}

int Scheduler::SendToHTTPWSThread(const ConnGUID* conn_guid, const proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len, int http_ws_thread_idx)
{
    return SendToThread(THREAD_TYPE_HTTP_WS, conn_guid, msg_head, msg_body, msg_body_len, http_ws_thread_idx);
}

int Scheduler::SendToThread(int thread_type, const ConnGUID* conn_guid, const proto::MsgHead& msg_head,
                            const void* msg_body, size_t msg_body_len, int thread_idx)
{
    ThreadGroupInterface* thread_group = nullptr;

    switch (thread_type)
    {
        case THREAD_TYPE_WORK:
        {
            thread_group = related_thread_groups_->work_thread_group;
        }
        break;

        case THREAD_TYPE_BURDEN:
        {
            thread_group = related_thread_groups_->burden_thread_group;
        }
        break;

        case THREAD_TYPE_RAW_TCP:
        {
            thread_group = related_thread_groups_->raw_tcp_thread_group;
        }
        break;

        case THREAD_TYPE_PROTO_TCP:
        {
            thread_group = related_thread_groups_->proto_tcp_thread_group;
        }
        break;

        case THREAD_TYPE_HTTP_WS:
        {
            thread_group = related_thread_groups_->http_ws_thread_group;
        }
        break;

        default:
        {
            LOG_ERROR("invalid thread type: " << thread_type);
            return -1;
        }
        break;
    }

    if (nullptr == thread_group)
    {
        LOG_ERROR("no such threads, thread type: " << thread_type);
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

    ThreadTask* task = new ThreadTask(TASK_TYPE_NORMAL, thread_sink_->GetThread(), conn_guid, data + TOTAL_MSG_LEN_FIELD_LEN, len - TOTAL_MSG_LEN_FIELD_LEN); // 内部的消息不发送4个字节的长度字段
    if (nullptr == task)
    {
        const int err = errno;
        LOG_ERROR("failed to create task, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    const int thread_count = thread_group->GetThreadCount();

    if (-1 == thread_idx)
    {
        for (int i = 0; i < thread_count; ++i)
        {
            ThreadInterface* thread = thread_group->GetThread(i);
            thread->PushTask(task->Clone());
        }

        task->Release();
        return 0;
    }

    if (INVALID_IDX(thread_idx, 0, thread_count))
    {
        LOG_ERROR("invalid thread idx: " << thread_idx << ", thread count: " << thread_count);

        task->Release();
        return -1;
    }

    ThreadInterface* thread = thread_group->GetThread(thread_idx);
    thread->PushTask(task);

    return 0;
}
}
