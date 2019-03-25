#ifndef RAW_TCP_THREADS_SRC_LISTEN_THREAD_SINK_H_
#define RAW_TCP_THREADS_SRC_LISTEN_THREAD_SINK_H_

// 单独的监听线程是有必要的，如果放在io线程中，一旦io处理逻辑慢了就会影响建立连接。

#include <event2/listener.h>
#include "new_conn.h"
#include "mem_util.h"
#include "raw_tcp_threads_interface.h"

namespace tcp
{
namespace raw
{
class ListenThreadSink : public ThreadSinkInterface
{
    CREATE_FUNC(ListenThreadSink)

private:
    static void ErrorCallback(struct evconnlistener* listener, void* arg);
    static void OnAccept(struct evconnlistener* listener, evutil_socket_t sock_fd,
                         struct sockaddr* sock_addr, int sock_addr_len, void* arg);

public:
    ListenThreadSink();
    virtual ~ListenThreadSink();

    ///////////////////////// ThreadSinkInterface /////////////////////////
    void Release() override;
    int OnInitialize(ThreadInterface* thread, const void* ctx) override;
    void OnFinalize() override;
    int OnActivate() override;
    void OnFreeze() override;
    void OnThreadStartOK() override;
    void OnStop() override;
    void OnReload() override;
    void OnTask(const ThreadTask* task) override;
    bool CanExit() const override;

    void SetTCPThreadGroup(ThreadGroupInterface* tcp_thread_group)
    {
        tcp_thread_group_ = tcp_thread_group;
    }

    void OnClientConnected(const NewConnCtx* new_conn_ctx);

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* tcp_thread_group_;
    struct evconnlistener* listener_;
    int online_tcp_conn_count_;
    int max_online_tcp_conn_count_;
    int tcp_thread_count_;
    int last_tcp_thread_idx_;
};
}
}

#endif // RAW_TCP_THREADS_SRC_LISTEN_THREAD_SINK_H_
