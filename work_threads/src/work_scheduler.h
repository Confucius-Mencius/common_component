#ifndef WORK_THREADS_SRC_WORK_SCHEDULER_H_
#define WORK_THREADS_SRC_WORK_SCHEDULER_H_

#include "thread_center_interface.h"
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

    ///////////////////////// base::SchedulerInterface /////////////////////////
    struct event_base* GetThreadEvBase() const override;
    int GetThreadIdx() const override;
    TransId SendToServer(const Peer& peer, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len,
                         const base::AsyncCtx* async_ctx) override;
    TransId HttpGet(const Peer& peer, const http::GetParams& params, const base::AsyncCtx* async_ctx) override;
    TransId HttpPost(const Peer& peer, const http::PostParams& params, const base::AsyncCtx* async_ctx) override;
    TransId HttpHead(const Peer& peer, const http::HeadParams& params, const base::AsyncCtx* async_ctx) override;
    void CancelTrans(TransId trans_id) override;

    ///////////////////////// SchedulerInterface /////////////////////////
    int SendToWorkThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                         size_t msg_body_len, int work_thread_idx) override;
    int SendToBurdenThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                           size_t msg_body_len, int burden_thread_idx) override;
    int SendToGlobalThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                           size_t msg_body_len) override;

    int SendToTcpClient(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                        size_t msg_body_len) override;
    int SendRawToTcpClient(const ConnGuid* conn_guid, const void* msg, size_t msg_len) override;
    int CloseTcpClient(const ConnGuid* conn_guid) override;

    int SendToUdpClient(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                        size_t msg_body_len) override;
    int CloseUdpClient(const ConnGuid* conn_guid) override;

    int SendToTcpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                        size_t msg_body_len, int tcp_thread_idx) override;
    int SendToHttpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                         size_t msg_body_len, int http_thread_idx) override;
    int SendToUdpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                        size_t msg_body_len, int udp_thread_idx) override;

public:
    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

    void SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group);

private:
    int GetScheduleWorkThreadIdx(int work_thread_idx);
    int GetScheduleBurdenThreadIdx(int burden_thread_idx);
    int GetScheduleTcpThreadIdx(int tcp_thread_idx);
    int GetScheduleHttpThreadIdx(int http_thread_idx);
    int GetScheduleUdpThreadIdx(int udp_thread_idx);

private:
    ThreadSink* thread_sink_;
    RelatedThreadGroup* related_thread_group_;

    int last_work_thread_idx_;
    int last_burden_thread_idx_;
    int last_tcp_thread_idx_;
    int last_http_thread_idx_;
    int last_udp_thread_idx_;
};
}

#endif // WORK_THREADS_SRC_WORK_SCHEDULER_H_
