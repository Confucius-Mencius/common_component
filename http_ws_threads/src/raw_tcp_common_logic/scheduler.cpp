#include "scheduler.h"
#include <memory>
#include "log_util.h"

namespace tcp
{
namespace http_ws
{
Scheduler::Scheduler()
{
    raw_tcp_scheduler_ = nullptr;
    msg_codec_ = nullptr;
}

Scheduler::~Scheduler()
{
}

int Scheduler::SendToClient(const ConnGUID* conn_guid, const void* data, size_t len)
{
    return raw_tcp_scheduler_->SendToClient(conn_guid, data, len);
}

int Scheduler::CloseClient(const ConnGUID* conn_guid)
{
    return raw_tcp_scheduler_->CloseClient(conn_guid);
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

int Scheduler::SendToTCPThread(const ConnGUID* conn_guid, const proto::MsgHead& msg_head,
                               const void* msg_body, size_t msg_body_len, int tcp_thread_idx)
{
    return SendToThread(THREAD_TYPE_TCP, conn_guid, msg_head, msg_body, msg_body_len, tcp_thread_idx);
}

int Scheduler::SendToThread(int thread_type, const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                            const void* msg_body, size_t msg_body_len, int thread_idx)
{
    switch (thread_type)
    {
        case THREAD_TYPE_GLOBAL:
        {
            return raw_tcp_scheduler_->SendToGlobalThread(conn_guid, msg_head, msg_body, msg_body_len);
        }
        break;

        case THREAD_TYPE_WORK:
        {
            return raw_tcp_scheduler_->SendToWorkThread(conn_guid, msg_head, msg_body, msg_body_len, thread_idx);
        }
        break;

        case THREAD_TYPE_TCP:
        {
            return raw_tcp_scheduler_->SendToTCPThread(conn_guid, msg_head, msg_body, msg_body_len, thread_idx);
        }
        break;

        default:
        {
            LOG_ERROR("invalid thread type: " << thread_type);
            return -1;
        }
        break;
    }

    return 0;
}
}
}
