#include "scheduler.h"
#include "log_util.h"

namespace global
{
Scheduler::Scheduler()
{
    work_scheduler_ = nullptr;
}

Scheduler::~Scheduler()
{
}

int Scheduler::SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                const void* msg_body, size_t msg_body_len, int work_thread_idx)
{
    return work_scheduler_->SendToWorkThread(conn_guid, msg_head, msg_body, msg_body_len, work_thread_idx);
}

int Scheduler::SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len, int burden_thread_idx)
{
    return work_scheduler_->SendToBurdenThread(conn_guid, msg_head, msg_body, msg_body_len, burden_thread_idx);
}

int Scheduler::SendToRawTCPThread(const ConnGUID* conn_guid, const proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len, int raw_tcp_thread_idx)
{
    return work_scheduler_->SendToRawTCPThread(conn_guid, msg_head, msg_body, msg_body_len, raw_tcp_thread_idx);
}

int Scheduler::SendToProtoTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                    const void* msg_body, size_t msg_body_len, int proto_tcp_thread_idx)
{
    return work_scheduler_->SendToProtoTCPThread(conn_guid, msg_head, msg_body, msg_body_len, proto_tcp_thread_idx);
}

int Scheduler::SendToHTTPWSThread(const ConnGUID* conn_guid, const proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len, int http_ws_thread_idx)
{
    return work_scheduler_->SendToProtoTCPThread(conn_guid, msg_head, msg_body, msg_body_len, http_ws_thread_idx);
}
}
