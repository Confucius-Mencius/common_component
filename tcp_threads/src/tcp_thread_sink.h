#ifndef TCP_THREADS_SRC_TCP_THREAD_SINK_H_
#define TCP_THREADS_SRC_TCP_THREAD_SINK_H_

#include <set>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
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
    int OnInitialize(ThreadInterface* thread) override;
    void OnFinalize() override;
    int OnActivate() override;
    void OnFreeze() override;
    void OnThreadStartOK() override;
    void OnStop() override;
    void OnReload() override;
    void OnTask(const ThreadTask* task) override;
    bool CanExit() const override;

public:
    void SetThreadsCtx(const ThreadsCtx* threads_ctx)
    {
        threads_ctx_ = threads_ctx;
    }

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

    void CloseConn(evutil_socket_t sock_fd);
    void OnClientClosed(BaseConn* conn);
    void OnConnTimeout(BaseConn* conn);

private:
    int LoadLocalLogic();
    int LoadLogicGroup();
    void OnClientConnected(const NewConnCtx* new_conn_ctx);

#if defined(USE_BUFFEREVENT)
    void OnRecvClientData(struct evbuffer* input_buf, int sock_fd, BaseConn* conn);
#else
    void OnClientData(bool& closed, int sock_fd, ConnInterface* conn);
    void OnClientRawData(bool& closed, int sock_fd, const ConnInterface* conn);
#endif

//    void ExhaustSocketData(int sock_fd);

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

#if !defined(USE_BUFFEREVENT)
    struct ConnRecvCtx
    {
        char total_msg_len_network_[TOTAL_MSG_LEN_FIELD_LEN];
        ssize_t total_msg_len_network_recved_len_;
        int32_t total_msg_len_;
        ssize_t total_msg_recved_len_;
        char* msg_recv_buf_;

        ConnRecvCtx()
        {
            total_msg_len_network_recved_len_ = 0;
            total_msg_len_ = 0;
            total_msg_recved_len_ = 0;
            msg_recv_buf_ = NULL;
        }
    };

    typedef __hash_map<int, ConnRecvCtx> ConnRecvCtxHashTable;
    ConnRecvCtxHashTable conn_recv_ctx_hash_table_;
#endif
};
}

#endif // TCP_THREADS_SRC_TCP_THREAD_SINK_H_
