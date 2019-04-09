#ifndef HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_SCHEDULER_H_
#define HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_SCHEDULER_H_

#include "http_ws_scheduler_interface.h"
#include "proto_msg_codec.h"
#include "raw_tcp_scheduler_interface.h"

namespace tcp
{
namespace http_ws
{
class ThreadSink;

class Scheduler : public SchedulerInterface
{
public:
    Scheduler();
    ~Scheduler();

    void SetRawTCPScheduler(tcp::raw::SchedulerInterface* scheduler)
    {
        raw_tcp_scheduler_ = scheduler;
    }

    void SetMsgCodec(::proto::MsgCodec* msg_codec)
    {
        msg_codec_ = msg_codec;
    }

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
    enum
    {
        THREAD_TYPE_GLOBAL,
        THREAD_TYPE_WORK,
        THREAD_TYPE_TCP,
    };

    int SendToThread(int thread_type, const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                     const void* msg_body, size_t msg_body_len, int thread_idx);

private:
    tcp::raw::SchedulerInterface* raw_tcp_scheduler_;
    ::proto::MsgCodec* msg_codec_;
};
}
}

#endif // HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_SCHEDULER_H_
