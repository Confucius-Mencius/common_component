#ifndef GLOBAL_THREADS_SRC_WORK_COMMON_LOGIC_SCHEDULER_H_
#define GLOBAL_THREADS_SRC_WORK_COMMON_LOGIC_SCHEDULER_H_

#include "global_scheduler_interface.h"
#include "work_scheduler_interface.h"
#include "work_threads_interface.h"

namespace global
{
class Scheduler : public SchedulerInterface
{
public:
    Scheduler();
    virtual ~Scheduler();

    void SetWorkScheduler(work::SchedulerInterface* scheduler)
    {
        work_scheduler_ = scheduler;
    }

    void SetRelatedThreadGroups(work::RelatedThreadGroups* related_thread_groups)
    {
        related_thread_groups_ = related_thread_groups;
    }

    ///////////////////////// SchedulerInterface /////////////////////////
    int SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                         const void* msg_body, size_t msg_body_len, int work_thread_idx) override;
    int SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                           const void* msg_body, size_t msg_body_len, int burden_thread_idx) override;
    int SendToRawTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                           const void* msg_body, size_t msg_body_len, int raw_tcp_thread_idx) override;
    int SendToProtoTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                             const void* msg_body, size_t msg_body_len, int proto_tcp_thread_idx) override;
    int SendToHTTPWSThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                           const void* msg_body, size_t msg_body_len, int http_ws_thread_idx) override;

private:
    work::SchedulerInterface* work_scheduler_;
    work::RelatedThreadGroups* related_thread_groups_;
};
}

#endif // GLOBAL_THREADS_SRC_WORK_COMMON_LOGIC_SCHEDULER_H_
