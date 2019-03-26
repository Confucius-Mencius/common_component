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

    return raw_tcp_scheduler_->SendToTCPThread(conn_guid, data, len, tcp_thread_idx);
}

int Scheduler::SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len, int work_thread_idx)
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

    return raw_tcp_scheduler_->SendToWorkThread(conn_guid, data, len, work_thread_idx);
}

int Scheduler::SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
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

    return raw_tcp_scheduler_->SendToGlobalThread(conn_guid, data, len);
}
}
}
