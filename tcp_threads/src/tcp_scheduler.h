#ifndef TCP_THREADS_SRC_TCP_SCHEDULER_H_
#define TCP_THREADS_SRC_TCP_SCHEDULER_H_

#include "tcp_scheduler_interface.h"
#include "tcp_threads_interface.h"

namespace tcp
{
class ThreadSink;

class Scheduler : public SchedulerInterface
{
public:
    Scheduler();
    virtual ~Scheduler();

    int Initialize(const void* ctx);
    void Finalize();

    ///////////////////////// SchedulerInterface /////////////////////////
    int SendToClient(const ConnGUID* conn_guid, const void* data, size_t len) override;
    int CloseClient(const ConnGUID* conn_guid) override;
    int SendToTCPThread(const ConnGUID* conn_guid, const void* data, size_t len, int tcp_thread_idx) override;
    int SendToWorkThread(const ConnGUID* conn_guid, const void* data, size_t len, int work_thread_idx) override;
    int SendToGlobalThread(const ConnGUID* conn_guid, const void* data, size_t len) override;

    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups);

private:
    int GetScheduleTCPThreadIdx(int tcp_thread_idx);
    int GetScheduleWorkThreadIdx(int work_thread_idx);

private:
    const ThreadsCtx* threads_ctx_;
    ThreadSink* thread_sink_;
    RelatedThreadGroups* related_thread_groups_;

    int last_tcp_thread_idx_;
    int last_work_thread_idx_;
};
}

#endif // TCP_THREADS_SRC_TCP_SCHEDULER_H_
