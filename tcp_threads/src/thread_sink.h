#ifndef TCP_THREADS_SRC_THREAD_SINK_H_
#define TCP_THREADS_SRC_THREAD_SINK_H_

#include <event2/listener.h>
#include "conn_center.h"
#include "mem_util.h"
#include "msg_dispatcher.h"
#include "module_loader.h"
#include "proto_msg_codec.h"
#include "tcp_logic_interface.h"
#include "scheduler.h"

namespace tcp
{
struct LogicItem
{
    std::string logic_so_path;
    ModuleLoader logic_loader;
    LogicInterface* logic;

    LogicItem() : logic_so_path(), logic_loader()
    {
        logic = nullptr;
    }
};

typedef std::vector<LogicItem> LogicItemVec;

class ThreadSink : public ThreadSinkInterface
{
    CREATE_FUNC(ThreadSink)

    static void OnAccept(struct evconnlistener* listener, evutil_socket_t sock_fd,
                         struct sockaddr* sock_addr, int sock_addr_len, void* arg);
    static void OnListenError(struct evconnlistener* listener, void* arg);

public:
    ThreadSink();
    virtual ~ThreadSink();

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

public:
    void SetTCPThreadGroup(ThreadGroupInterface* tcp_thread_group)
    {
        tcp_thread_group_ = tcp_thread_group;
    }

    ThreadGroupInterface* GetTCPThreadGroup()
    {
        return tcp_thread_group_;
    }

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups);

    ConnCenter* GetConnCenter()
    {
        return &conn_center_;
    }

    void OnClientClosed(const BaseConn* conn);
    void OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len);

private:
    struct NewConnCtx
    {
        char client_ip[INET_ADDRSTRLEN];
        unsigned short client_port;
        int client_sock_fd;

        NewConnCtx()
        {
            client_ip[0] = '\0';
            client_port = 0;
            client_sock_fd = -1;
        }
    };

private:
    int LoadCommonLogic();
    int LoadLogicGroup();
    int OnClientConnected(const NewConnCtx* new_conn_ctx);

private:
    const ThreadsCtx* threads_ctx_;
    struct evconnlistener* listener_;
    int online_tcp_conn_count_;
    int max_online_tcp_conn_count_;
    ThreadGroupInterface* tcp_thread_group_;
    RelatedThreadGroups* related_thread_groups_;

    ModuleLoader common_logic_loader_;
    CommonLogicInterface* common_logic_;
    LogicItemVec logic_item_vec_;

    ConnCenter conn_center_;

    ::proto::MsgCodec msg_codec_;
    Scheduler scheduler_;
    MsgDispatcher msg_dispatcher_;
};
}

#endif // TCP_THREADS_SRC_THREAD_SINK_H_
