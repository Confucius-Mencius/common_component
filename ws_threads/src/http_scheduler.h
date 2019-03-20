#ifndef WS_THREADS_SRC_HTTP_SCHEDULER_H_
#define WS_THREADS_SRC_HTTP_SCHEDULER_H_

#include "ws_scheduler_interface.h"
#include "ws_threads_interface.h"

namespace ws
{
class ThreadSink;

namespace http
{
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
    int SendToWSThread(const ConnGUID* conn_guid, const void* data, size_t len, int ws_thread_idx) override;
    int SendToWorkThread(const ConnGUID* conn_guid, const void* data, size_t len, int work_thread_idx) override;
    int SendToGlobalThread(const ConnGUID* conn_guid, const void* data, size_t len) override;

    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups);

private:
    int GetScheduleWSThreadIdx(int ws_thread_idx);
    int GetScheduleWorkThreadIdx(int work_thread_idx);

private:
    const ThreadsCtx* threads_ctx_;
    ThreadSink* thread_sink_;
    RelatedThreadGroups* related_thread_groups_;

    int last_ws_thread_idx_;
    int last_work_thread_idx_;
};
}
}

#endif // WS_THREADS_SRC_HTTP_SCHEDULER_H_
