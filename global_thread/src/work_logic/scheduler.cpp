#include "scheduler.h"

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
    if (-1 == work_thread_idx)
    {
        // 广播
        for (int i = 0; i < related_thread_groups_->work_thread_group->GetThreadCount(); ++i)
        {
            work_scheduler_->SendToWorkThread(conn_guid, msg_head, msg_body, msg_body_len, i);
        }

        return 0;
    }
    else
    {
        return work_scheduler_->SendToWorkThread(conn_guid, msg_head, msg_body, msg_body_len, work_thread_idx);
    }
}

int Scheduler::SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                  const void* msg_body, size_t msg_body_len, int burden_thread_idx)
{
    if (-1 == burden_thread_idx)
    {
        // 广播
        for (int i = 0; i < related_thread_groups_->burden_thread_group->GetThreadCount(); ++i)
        {
            work_scheduler_->SendToBurdenThread(conn_guid, msg_head, msg_body, msg_body_len, i);
        }

        return 0;
    }
    else
    {
        return work_scheduler_->SendToBurdenThread(conn_guid, msg_head, msg_body, msg_body_len, burden_thread_idx);
    }
}

int Scheduler::SendToTCPThread(const ConnGUID* conn_guid, const proto::MsgHead& msg_head,
                               const void* msg_body, size_t msg_body_len, int tcp_thread_idx)
{
    if (-1 == tcp_thread_idx)
    {
        // 广播
        for (int i = 0; i < related_thread_groups_->tcp_thread_group->GetThreadCount(); ++i)
        {
            work_scheduler_->SendToTCPThread(conn_guid, msg_head, msg_body, msg_body_len, i);
        }

        return 0;
    }
    else
    {
        return work_scheduler_->SendToTCPThread(conn_guid, msg_head, msg_body, msg_body_len, tcp_thread_idx);
    }
}

int Scheduler::SendToProtoTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                    const void* msg_body, size_t msg_body_len, int proto_tcp_thread_idx)
{
    if (-1 == proto_tcp_thread_idx)
    {
        // 广播
        for (int i = 0; i < related_thread_groups_->proto_tcp_thread_group->GetThreadCount(); ++i)
        {
            work_scheduler_->SendToProtoTCPThread(conn_guid, msg_head, msg_body, msg_body_len, i);
        }

        return 0;
    }
    else
    {
        return work_scheduler_->SendToProtoTCPThread(conn_guid, msg_head, msg_body, msg_body_len, proto_tcp_thread_idx);
    }
}

int Scheduler::SendToWebThread(const ConnGUID* conn_guid, const proto::MsgHead& msg_head,
                               const void* msg_body, size_t msg_body_len, int web_thread_idx)
{
    if (-1 == web_thread_idx)
    {
        // 广播
        for (int i = 0; i < related_thread_groups_->web_thread_group->GetThreadCount(); ++i)
        {
            work_scheduler_->SendToWebThread(conn_guid, msg_head, msg_body, msg_body_len, i);
        }

        return 0;
    }
    else
    {
        return work_scheduler_->SendToWebThread(conn_guid, msg_head, msg_body, msg_body_len, web_thread_idx);
    }
}
}
