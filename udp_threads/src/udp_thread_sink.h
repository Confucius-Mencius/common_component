#ifndef UDP_THREADS_SRC_UDP_THREAD_SINK_H_
#define UDP_THREADS_SRC_UDP_THREAD_SINK_H_

#include <event2/event.h>
#include "conn_center_mgr_interface.h"
#include "client_center_mgr_interface.h"
#include "mem_util.h"
#include "module_loader.h"
#include "thread_center_interface.h"
#include "udp_logic_interface.h"
#include "udp_scheduler.h"
#include "udp_threads_interface.h"

namespace udp
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

class ThreadSink : public ThreadSinkInterface, public tcp::NfySinkInterface, public ConnTimeoutSinkInterface
{
    CREATE_FUNC(ThreadSink);
    static void ReadCallback(evutil_socket_t sock, short events, void* arg);

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

    ///////////////////////// tcp::NfySinkInterface /////////////////////////
    void OnRecvNfy(const Peer& peer, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len) override;

    ///////////////////////// ConnTimeoutSinkInterface /////////////////////////
    void OnConnTimeout(ConnInterface* conn) override;

public:
    void SetThreadsCtx(const ThreadsCtx* threads_ctx)
    {
        threads_ctx_ = threads_ctx;
    }

    void SetUdpThreadGroup(ThreadGroupInterface* udp_thread_group)
    {
        udp_thread_group_ = udp_thread_group;
    }

    char* GetRecvBuf() const
    {
        return recv_buf_;
    }

    void OnRecvClientMsg(evutil_socket_t fd, const struct sockaddr_in* client_addr, const char* total_msg_buf,
                         size_t total_msg_len);

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

    ThreadGroupInterface* GetUdpThreadGroup()
    {
        return udp_thread_group_;
    }

    void SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group);

private:
    int BindUdpSocket();
    int LoadLocalLogic();
    int LoadLogicGroup();

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* udp_thread_group_;

    ConnCenterInterface* conn_center_;
    MsgCodecInterface* udp_msg_codec_;

    tcp::ClientCenterInterface* tcp_client_center_;
    http::ClientCenterInterface* http_client_center_;
    udp::ClientCenterInterface* udp_client_center_;

    Scheduler scheduler_;

    int listen_sock_fd_;
    struct event* udp_event_;

    char* recv_buf_;

    ModuleLoader local_logic_loader_;
    LocalLogicInterface* local_logic_;

    LogicItemVec logic_item_vec_;

    RelatedThreadGroup* related_thread_group_;
};
}

#endif // UDP_THREADS_SRC_UDP_THREAD_SINK_H_
