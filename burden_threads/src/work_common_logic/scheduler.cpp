#include "scheduler.h"

namespace burden
{
Scheduler::Scheduler()
{
    work_scheduler_ = nullptr;
}

Scheduler::~Scheduler()
{
}

int Scheduler::SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len)
{
    return work_scheduler_->SendToGlobalThread(conn_guid, msg_head, msg_body, msg_body_len);
}

int Scheduler::SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len, int burden_thread_idx)
{
    return work_scheduler_->SendToBurdenThread(conn_guid, msg_head, msg_body, msg_body_len, burden_thread_idx);
}
}
