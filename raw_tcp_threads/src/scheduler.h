#ifndef RAW_TCP_THREADS_SRC_SCHEDULER_H_
#define RAW_TCP_THREADS_SRC_SCHEDULER_H_

#include "raw_tcp_scheduler_interface.h"
#include "raw_tcp_threads_interface.h"

namespace tcp
{
namespace raw
{
class IOThreadSink;

class Scheduler : public SchedulerInterface
{
public:
    Scheduler();
    virtual ~Scheduler();

    void SetThreadSink(IOThreadSink* sink)
    {
        thread_sink_ = sink;
    }

    int Initialize(const void* ctx);
    void Finalize();

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups);

    ///////////////////////// SchedulerInterface /////////////////////////
    int SendToClient(const ConnGUID* conn_guid, const void* data, size_t len) override;
    int CloseClient(const ConnGUID* conn_guid) override;
    int SendToGlobalThread(const ConnGUID* conn_guid, const void* data, size_t len) override;
    int SendToTCPThread(const ConnGUID* conn_guid, const void* data, size_t len, int tcp_thread_idx) override;
    int SendToWorkThread(const ConnGUID* conn_guid, const void* data, size_t len, int work_thread_idx) override;

private:
    int GetScheduleTCPThreadIdx(int tcp_thread_idx);
    int GetScheduleWorkThreadIdx(int work_thread_idx);

    enum
    {
        THREAD_TYPE_GLOBAL,
        THREAD_TYPE_TCP,
        THREAD_TYPE_WORK,
    };

    int SendToThread(int thread_type, const ConnGUID* conn_guid, const void* data, size_t len, int thread_idx);

private:
    IOThreadSink* thread_sink_;
    const ThreadsCtx* threads_ctx_;
    RelatedThreadGroups* related_thread_groups_;

    int last_tcp_thread_idx_;
    int last_work_thread_idx_;
};
}
}

#endif // RAW_TCP_THREADS_SRC_SCHEDULER_H_
