#ifndef RAW_TCP_THREADS_SRC_SCHEDULER_H_
#define RAW_TCP_THREADS_SRC_SCHEDULER_H_

#include "proto_msg_codec.h"
#include "raw_tcp_scheduler_interface.h"
#include "raw_tcp_threads_interface.h"

namespace tcp
{
namespace raw
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
    int SendToClient(const ConnGUID* conn_guid, const void* data, size_t len) override;
    int CloseClient(const ConnGUID* conn_guid) override;
    int SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                           const void* msg_body, size_t msg_body_len) override;
    int SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                         const void* msg_body, size_t msg_body_len, int work_thread_idx) override;
    int SendToTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                        const void* msg_body, size_t msg_body_len, int tcp_thread_idx) override;

private:
    int GetScheduleWorkThreadIdx(int work_thread_idx);
    int GetScheduleTCPThreadIdx(int tcp_thread_idx);

    enum
    {
        THREAD_TYPE_GLOBAL,
        THREAD_TYPE_WORK,
        THREAD_TYPE_TCP,
    };

    int SendToThread(int thread_type, const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                     const void* msg_body, size_t msg_body_len, int thread_idx);

private:
    ThreadSink* thread_sink_;
    ::proto::MsgCodec* msg_codec_;
    const ThreadsCtx* threads_ctx_;
    RelatedThreadGroups* related_thread_groups_;

    int last_work_thread_idx_;
    int last_tcp_thread_idx_;
};
}
}

#endif // RAW_TCP_THREADS_SRC_SCHEDULER_H_
