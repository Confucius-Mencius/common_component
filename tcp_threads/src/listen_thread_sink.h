#ifndef TCP_THREADS_SRC_LISTEN_THREAD_SINK_H_
#define Tcp_THREADS_SRC_LISTEN_THREAD_SINK_H_

// 单独的监听线程是有必要的，如果放在io线程中，如果io处理逻辑慢了就会影响建立连接。

#include <event2/listener.h>
#include "inner_define.h"
#include "mem_util.h"
#include "tcp_threads_interface.h"
#include "thread_center_interface.h"

namespace tcp
{
class ListenThreadSink : public ThreadSinkInterface
{
    CREATE_FUNC(ListenThreadSink);
    static void ErrorCallback(struct evconnlistener* listener, void* arg);
    static void OnAccept(struct evconnlistener* listener, evutil_socket_t sock_fd,
                         struct sockaddr* sock_addr, int sock_addr_len, void* arg);

public:
    ListenThreadSink();
    virtual ~ListenThreadSink();

    ///////////////////////// ThreadSinkInterface /////////////////////////
    void Release() override;
    int OnInitialize(ThreadInterface* thread) override;
    void OnFinalize() override;
    int OnActivate() override;
    void OnFreeze() override;
    void OnThreadStartOk() override;
    void OnStop() override;
    void OnReload() override;
    void OnTask(const Task* task) override;
    bool CanExit() const override;

public:
    void SetThreadsCtx(const ThreadsCtx* threads_ctx)
    {
        threads_ctx_ = threads_ctx;
    }

    void SetTcpThreadGroup(ThreadGroupInterface* tcp_thread_group)
    {
        tcp_thread_group_ = tcp_thread_group;
    }

    void OnClientConnected(const NewConnCtx* new_conn_ctx);

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* tcp_thread_group_;
    evutil_socket_t listen_sock_fd_;
    struct evconnlistener* listener_;
    int last_tcp_thread_idx_;
    int online_tcp_conn_count_;
    int max_online_tcp_conn_count_;
};
}

#endif // TCP_THREADS_SRC_LISTEN_THREAD_SINK_H_
