#ifndef GLOBAL_THREAD_SRC_SCHEDULER_H_
#define GLOBAL_THREAD_SRC_SCHEDULER_H_

#include "global_scheduler_interface.h"
#include "global_threads_interface.h"
#include "proto_msg_codec.h"
#include "thread_center_interface.h"

namespace global
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

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups)
    {
        related_thread_groups_ = related_thread_groups;
    }

    void SetMsgCodec(::proto::MsgCodec* msg_codec)
    {
        msg_codec_ = msg_codec;
    }

    ///////////////////////// SchedulerInterface /////////////////////////
    int SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                         size_t msg_body_len, int work_thread_idx) override;
    int SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                           size_t msg_body_len, int burden_thread_idx) override;
    int SendToProtoTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                             size_t msg_body_len, int proto_tcp_thread_idx) override;

private:
    enum
    {
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
};
}

#endif // GLOBAL_THREAD_SRC_SCHEDULER_H_
