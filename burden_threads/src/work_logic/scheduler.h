#ifndef BURDEN_THREADS_SRC_WORK_LOGIC_SCHEDULER_H_
#define BURDEN_THREADS_SRC_WORK_LOGIC_SCHEDULER_H_

#include "burden_scheduler_interface.h"
#include "work_scheduler_interface.h"

namespace burden
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

    ///////////////////////// SchedulerInterface /////////////////////////
    int SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                           const void* msg_body, size_t msg_body_len) override;
    int SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                           const void* msg_body, size_t msg_body_len, int burden_thread_idx) override;

private:
    work::SchedulerInterface* work_scheduler_;
};
}

#endif // BURDEN_THREADS_SRC_WORK_LOGIC_SCHEDULER_H_
