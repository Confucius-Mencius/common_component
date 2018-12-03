#ifndef HTTP_THREADS_SRC_HTTP_SCHEDULER_H_
#define HTTP_THREADS_SRC_HTTP_SCHEDULER_H_

#include "http_scheduler_interface.h"
#include "http_threads_interface.h"
#include "thread_center_interface.h"

namespace http
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
    int SendToClient(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, int http_code,
                     const KeyValMap* http_header_map, const char* content, size_t content_len) override;
    TransId SendToWorkThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                             size_t msg_body_len, int work_thread_idx, const base::AsyncCtx* async_ctx) override;
    TransId SendToGlobalThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                               size_t msg_body_len, const base::AsyncCtx* async_ctx) override;

public:
    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

    void SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group);

private:
    int GetScheduleWorkThreadIdx(int work_thread_idx);

private:
    const ThreadsCtx* threads_ctx_;
    ThreadSink* thread_sink_;
    RelatedThreadGroup* related_thread_group_;

    int last_work_thread_idx_;
};
}

#endif // HTTP_THREADS_SRC_HTTP_SCHEDULER_H_
