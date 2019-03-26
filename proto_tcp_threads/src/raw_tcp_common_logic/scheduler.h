#ifndef PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_SCHEDULER_H_
#define PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_SCHEDULER_H_

#include "proto_msg_codec_interface.h"
#include "proto_tcp_scheduler_interface.h"
#include "raw_tcp_scheduler_interface.h"

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

//    int Initialize(const void* ctx);
//    void Finalize();

    void SetRawTCPScheduler(tcp::raw::SchedulerInterface* scheduler)
    {
        raw_tcp_scheduler_ = scheduler;
    }

    void SetMsgCodec(::proto::MsgCodecInterface* msg_codec)
    {
        msg_codec_ = msg_codec;
    }

    ///////////////////////// SchedulerInterface /////////////////////////
    int SendToClient(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                     size_t msg_body_len) override;
    int SendToClient(const ConnGUID* conn_guid, const void* data, size_t len) override;
    int CloseClient(const ConnGUID* conn_guid) override;
    int SendToTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                        size_t msg_body_len, int tcp_thread_idx) override;
    int SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                         size_t msg_body_len, int work_thread_idx) override;
    int SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                           size_t msg_body_len) override;

private:
    tcp::raw::SchedulerInterface* raw_tcp_scheduler_;
    ::proto::MsgCodecInterface* msg_codec_;
};
}
}

#endif // PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_SCHEDULER_H_
