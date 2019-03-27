#include "scheduler.h"
#include <memory>
#include "log_util.h"

namespace tcp
{
namespace proto
{
Scheduler::Scheduler()
{
    raw_tcp_scheduler_ = NULL;
    msg_codec_ = NULL;
}

Scheduler::~Scheduler()
{
}

int Scheduler::SendToClient(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
{
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

    return raw_tcp_scheduler_->SendToClient(conn_guid, data, len);
}

int Scheduler::SendToClient(const ConnGUID* conn_guid, const void* data, size_t len)
{
    return raw_tcp_scheduler_->SendToClient(conn_guid, data, len);
}

int Scheduler::CloseClient(const ConnGUID* conn_guid)
{
    return raw_tcp_scheduler_->CloseClient(conn_guid);
}

int Scheduler::SendToTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len, int tcp_thread_idx)
{
    return SendToThread(THREAD_TYPE_TCP, conn_guid, msg_head, msg_body, msg_body_len, tcp_thread_idx);
}

int Scheduler::SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len, int work_thread_idx)
{
    return SendToThread(THREAD_TYPE_WORK, conn_guid, msg_head, msg_body, msg_body_len, work_thread_idx);
}

int Scheduler::SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
{
    return SendToThread(THREAD_TYPE_GLOBAL, conn_guid, msg_head, msg_body, msg_body_len, 0);
}

int Scheduler::SendToThread(int thread_type, const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len, int thread_idx)
{
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

    switch (thread_type)
    {
        case THREAD_TYPE_TCP:
        {
            return raw_tcp_scheduler_->SendToTCPThread(conn_guid,
                    data + TOTAL_MSG_LEN_FIELD_LEN, len - TOTAL_MSG_LEN_FIELD_LEN, thread_idx); // 内部的消息不发送4个字节的长度字段
        }
        break;

        case THREAD_TYPE_WORK:
        {
            return raw_tcp_scheduler_->SendToWorkThread(conn_guid,
                    data + TOTAL_MSG_LEN_FIELD_LEN, len - TOTAL_MSG_LEN_FIELD_LEN, thread_idx); // 内部的消息不发送4个字节的长度字段
        }
        break;

        case THREAD_TYPE_GLOBAL:
        {
            return raw_tcp_scheduler_->SendToGlobalThread(conn_guid, data + TOTAL_MSG_LEN_FIELD_LEN, len - TOTAL_MSG_LEN_FIELD_LEN); // 内部的消息不发送4个字节的长度字段
        }
        break;

        default:
        {
        }
        break;
    }

    return 0;
}
}
}
