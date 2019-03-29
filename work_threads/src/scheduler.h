#ifndef WORK_THREADS_SRC_SCHEDULER_H_
#define WORK_THREADS_SRC_SCHEDULER_H_

#include "proto_msg_codec.h"
#include "work_scheduler_interface.h"
#include "work_threads_interface.h"

namespace work
{
class ThreadSink;

class Scheduler : public SchedulerInterface
{
public:
    Scheduler();
    virtual ~Scheduler();

    int Initialize(const void* ctx);
    void Finalize();

    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups);

    void SetMsgCodec(::proto::MsgCodec* msg_codec)
    {
        msg_codec_ = msg_codec;
    }

    ///////////////////////// SchedulerInterface /////////////////////////
    int SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                           size_t msg_body_len) override;
    int SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                         size_t msg_body_len, int work_thread_idx) override;
    int SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                           size_t msg_body_len, int burden_thread_idx) override;
    int SendToProtoTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                             size_t msg_body_len, int tcp_thread_idx) override;
    int SendToClient(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len);
    int SendToClient(const ConnGUID* conn_guid, const void* data, size_t len);
    int CloseClient(const ConnGUID* conn_guid);

private:
    int GetScheduleWorkThreadIdx(int work_thread_idx);
    int GetScheduleBurdenThreadIdx(int burden_thread_idx);
    int GetScheduleProtoTCPThreadIdx(int proto_tcp_thread_idx);

    enum
    {
        THREAD_TYPE_GLOBAL,
        THREAD_TYPE_WORK,
        THREAD_TYPE_BURDEN,
        THREAD_TYPE_PROTO_TCP,
    };

    int SendToThread(int thread_type, const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                     const void* msg_body, size_t msg_body_len, int thread_idx);

private:
    ThreadSink* thread_sink_;
    RelatedThreadGroups* related_thread_groups_;
    ::proto::MsgCodec* msg_codec_;

    int last_work_thread_idx_;
    int last_burden_thread_idx_;
    int last_proto_tcp_thread_idx_;
};
}

#endif // WORK_THREADS_SRC_SCHEDULER_H_
