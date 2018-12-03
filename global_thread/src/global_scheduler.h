#ifndef GLOBAL_THREAD_SRC_GLOBAL_SCHEDULER_H_
#define GLOBAL_THREAD_SRC_GLOBAL_SCHEDULER_H_

#include "global_scheduler_interface.h"
#include "global_threads_interface.h"
#include "thread_center_interface.h"

namespace global
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
    int SendToTcpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                        size_t msg_body_len, int tcp_thread_idx) override;
    int SendToHttpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                         size_t msg_body_len, int http_thread_idx) override;
    int SendToUdpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                        size_t msg_body_len, int udp_thread_idx) override;

public:
    void SetGlobalThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

    void SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group)
    {
        related_thread_group_ = related_thread_group;
    }

private:
    ThreadSink* thread_sink_;
    RelatedThreadGroup* related_thread_group_;
};
}

#endif // GLOBAL_THREAD_SRC_GLOBAL_SCHEDULER_H_
