#ifndef PROTO_TCP_THREADS_SRC_TCP_LOGIC_SCHEDULER_H_
#define PROTO_TCP_THREADS_SRC_TCP_LOGIC_SCHEDULER_H_

#include "proto_msg_codec.h"
#include "proto_tcp_scheduler_interface.h"
#include "tcp_scheduler_interface.h"

namespace tcp
{
namespace proto
{
class ThreadSink;

class Scheduler : public SchedulerInterface
{
public:
    Scheduler();
    ~Scheduler();

    void SetTCPScheduler(tcp::SchedulerInterface* scheduler)
    {
        tcp_scheduler_ = scheduler;
    }

    void SetMsgCodec(::proto::MsgCodec* msg_codec)
    {
        msg_codec_ = msg_codec;
    }

    ///////////////////////// SchedulerInterface /////////////////////////
    int SendToClient(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                     const void* msg_body, size_t msg_body_len) override;
    int SendToClient(const ConnGUID* conn_guid, const void* data, size_t len) override;
    int CloseClient(const ConnGUID* conn_guid) override;
    int SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                           const void* msg_body, size_t msg_body_len) override;
    int SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                         const void* msg_body, size_t msg_body_len, int work_thread_idx) override;
    int SendToProtoTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                        const void* msg_body, size_t msg_body_len, int tcp_thread_idx) override;

private:
    tcp::SchedulerInterface* tcp_scheduler_;
    ::proto::MsgCodec* msg_codec_;
};
}
}

#endif // PROTO_TCP_THREADS_SRC_TCP_LOGIC_SCHEDULER_H_
