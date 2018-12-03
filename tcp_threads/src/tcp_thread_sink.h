#ifndef TCP_THREADS_SRC_TCP_THREAD_SINK_H_
#define TCP_THREADS_SRC_TCP_THREAD_SINK_H_

#include <set>
#include <event2/buffer.h>
#include <event2/util.h>
#include "client_center_mgr_interface.h"
#include "conn_center_mgr_interface.h"
#include "inner_define.h"
#include "mem_util.h"
#include "module_loader.h"
#include "part_msg_mgr.h"
#include "tcp_logic_interface.h"
#include "tcp_scheduler.h"
#include "tcp_threads_interface.h"
#include "thread_center_interface.h"

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

class ThreadSink : public ThreadSinkInterface, public NfySinkInterface, public ConnTimeoutSinkInterface
{
    CREATE_FUNC(ThreadSink);

#if defined(USE_BUFFEREVENT)
    static void BufferEventEventCallback(struct bufferevent* buf_event, short events, void* arg);
    static void BufferEventReadCallback(struct bufferevent* buf_event, void* arg);
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
    void OnThreadStartOk() override;
    void OnStop() override;
    void OnReload() override;
    void OnTask(const Task* task) override;
    bool CanExit() const override;

    ///////////////////////// NfySinkInterface /////////////////////////
    void OnRecvNfy(const Peer& peer, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len) override;

    ///////////////////////// ConnTimeoutSinkInterface /////////////////////////
    void OnConnTimeout(ConnInterface* conn) override;

public:
    void SetThreadsCtx(const ThreadsCtx* threads_ctx)
    {
        threads_ctx_ = threads_ctx;
    }

    void SetListenThread(ThreadInterface* listen_thread)
    {
        listen_thread_ = listen_thread;
    }

    void SetTcpThreadGroup(ThreadGroupInterface* tcp_thread_group)
    {
        tcp_thread_group_ = tcp_thread_group;
    }

    void CloseConn(evutil_socket_t sock_fd);
    void OnClientClosed(ConnInterface* conn);
    void OnRecvClientMsg(const ConnGuid* conn_guid, const MsgHead& msg_head,
                         const void* msg_body, size_t msg_body_len);

    ConnCenterInterface* GetConnCenter()
    {
        return conn_center_;
    }

    tcp::ClientCenterInterface* GetTcpClientCenter()
    {
        return tcp_client_center_;
    }

    http::ClientCenterInterface* GetHttpClientCenter()
    {
        return http_client_center_;
    }

    udp::ClientCenterInterface* GetUdpClientCenter()
    {
        return udp_client_center_;
    }

    ThreadGroupInterface* GetTcpThreadGroup()
    {
        return tcp_thread_group_;
    }

    void SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group);

private:
    int LoadLocalLogic();
    int LoadLogicGroup();
    void OnClientConnected(const NewConnCtx* new_conn_ctx);

#if defined(USE_BUFFEREVENT)
    void OnClientData(struct evbuffer* input_buf, const int sock_fd, ConnInterface* conn);
    void OnClientRawData(struct evbuffer* input_buf, const int sock_fd, const ConnInterface* conn);
#else
    void OnClientData(bool& closed, int sock_fd, ConnInterface* conn);
    void OnClientRawData(bool& closed, int sock_fd, const ConnInterface* conn);
#endif

    void ExhaustSocketData(int sock_fd);

private:
    const ThreadsCtx* threads_ctx_;
    ThreadInterface* listen_thread_;
    ThreadGroupInterface* tcp_thread_group_;

    ConnCenterInterface* conn_center_;
    MsgCodecInterface* tcp_msg_codec_;

    tcp::ClientCenterInterface* tcp_client_center_;
    http::ClientCenterInterface* http_client_center_;
    udp::ClientCenterInterface* udp_client_center_;

    Scheduler scheduler_;

    char* msg_recv_buf_;
    size_t max_msg_recv_len_;

    PartMsgMgr part_msg_mgr_;

    ModuleLoader local_logic_loader_;
    LocalLogicInterface* local_logic_;

    LogicItemVec logic_item_vec_;

    RelatedThreadGroup* related_thread_group_;

    typedef std::map<std::string, int> PreClientBlacklistMap;
    PreClientBlacklistMap pre_client_blacklist_map_;

//    typedef std::set<std::string> ClientBlacklistSet;
//    ClientBlacklistSet client_blacklist_set_;

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
};
}

#endif // TCP_THREADS_SRC_TCP_THREAD_SINK_H_
