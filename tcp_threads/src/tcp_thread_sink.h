#ifndef TCP_THREADS_SRC_TCP_THREAD_SINK_H_
#define TCP_THREADS_SRC_TCP_THREAD_SINK_H_

#include <set>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event_struct.h>
#include <event2/util.h>
#include "new_conn.h"
#include "mem_util.h"
#include "module_loader.h"
#include "tcp_conn_mgr.h"
#include "tcp_logic_interface.h"
#include "tcp_scheduler.h"

namespace tcp
{
struct LogicItem
{
    std::string logic_so_path;
    ModuleLoader logic_loader;
    LogicInterface* logic;

    LogicItem() : logic_so_path(), logic_loader()
    {
        logic = NULL;
    }
};

typedef std::vector<LogicItem> LogicItemVec;

class ThreadSink : public ThreadSinkInterface
{
    CREATE_FUNC(ThreadSink)

private:
#if defined(USE_BUFFEREVENT)
    static void BufferEventEventCallback(struct bufferevent* buffer_event, short events, void* arg);
    static void BufferEventReadCallback(struct bufferevent* buffer_event, void* arg);
#else
    static void NormalReadCallback(evutil_socket_t fd, short events, void* arg);
#endif

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
    void SetListenThread(ThreadInterface* listen_thread)
    {
        listen_thread_ = listen_thread;
    }

    void SetTCPThreadGroup(ThreadGroupInterface* tcp_thread_group)
    {
        tcp_thread_group_ = tcp_thread_group;
    }

    ThreadGroupInterface* GetTCPThreadGroup()
    {
        return tcp_thread_group_;
    }

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups);

    ConnMgr* GetConnMgr()
    {
        return &conn_mgr_;
    }

    void OnClientClosed(const BaseConn* conn);

private:
    int LoadLocalLogic();
    int LoadLogicGroup();
    int OnClientConnected(const NewConnCtx* new_conn_ctx);

#if defined(USE_BUFFEREVENT)
    void OnRecvClientData(struct evbuffer* input_buf, int sock_fd, BaseConn* conn);
#else
    void OnRecvClientData(bool& closed, int sock_fd, BaseConn* conn);
#endif

private:
    const ThreadsCtx* threads_ctx_;
    ThreadInterface* listen_thread_;
    ThreadGroupInterface* tcp_thread_group_;
    RelatedThreadGroups* related_thread_group_;

    ModuleLoader local_logic_loader_;
    LocalLogicInterface* local_logic_;
    LogicItemVec logic_item_vec_;

    ConnMgr conn_mgr_;
    Scheduler scheduler_;
};
}

#endif // TCP_THREADS_SRC_TCP_THREAD_SINK_H_
