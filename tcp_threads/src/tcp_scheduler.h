#ifndef TCP_THREADS_SRC_TCP_SCHEDULER_H_
#define TCP_THREADS_SRC_TCP_SCHEDULER_H_

#include "tcp_scheduler_interface.h"
#include "tcp_threads_interface.h"
#include "thread_center_interface.h"

namespace tcp
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
    int SendToClient(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                     size_t msg_body_len) override;
    int SendRawToClient(const ConnGuid* conn_guid, const void* msg, size_t msg_len) override;
    int CloseClient(const ConnGuid* conn_guid) override;
    int SendToTcpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                        size_t msg_body_len, int tcp_thread_idx) override;
    int SendToWorkThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                         size_t msg_body_len, int work_thread_idx) override;
    int SendToGlobalThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                           size_t msg_body_len) override;

public:
    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

    void SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group);

private:
    int GetScheduleTcpThreadIdx(int tcp_thread_idx);
    int GetScheduleWorkThreadIdx(int work_thread_idx);

private:
    ThreadSink* thread_sink_;
    RelatedThreadGroup* related_thread_group_;

    int last_tcp_thread_idx_;
    int last_work_thread_idx_;
};
}

#endif // TCP_THREADS_SRC_TCP_SCHEDULER_H_
