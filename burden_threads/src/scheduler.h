#ifndef BURDEN_THREADS_SRC_SCHEDULER_H_
#define BURDEN_THREADS_SRC_SCHEDULER_H_

#include "burden_scheduler_interface.h"
#include "burden_threads_interface.h"
#include "proto_msg_codec.h"

namespace burden
{
class ThreadSink;

class Scheduler : public SchedulerInterface
{
public:
    Scheduler();
    virtual ~Scheduler();

    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

    void SetMsgCodec(::proto::MsgCodec* msg_codec)
    {
        msg_codec_ = msg_codec;
    }

    int Initialize(const void* ctx);
    void Finalize();

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups);

    ///////////////////////// SchedulerInterface /////////////////////////
    int SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                           size_t msg_body_len) override;
    int SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                           size_t msg_body_len, int burden_thread_idx) override;
    // int SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
    //                      size_t msg_body_len, int work_thread_idx) override;

private:
    int GetScheduleBurdenThreadIdx(int burden_thread_idx);
    // int GetScheduleWorkThreadIdx(int work_thread_idx);

    enum
    {
        THREAD_TYPE_WORK,
        THREAD_TYPE_BURDEN,
        THREAD_TYPE_GLOBAL,
    };

    int SendToThread(int thread_type, const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                     const void* msg_body, size_t msg_body_len, int thread_idx);

private:
    ThreadSink* thread_sink_;
    ::proto::MsgCodec* msg_codec_;
    const ThreadsCtx* threads_ctx_;
    RelatedThreadGroups* related_thread_groups_;

    // int last_work_thread_idx_;
    int last_burden_thread_idx_;
};
}

#endif // BURDEN_THREADS_SRC_SCHEDULER_H_
