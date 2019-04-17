#ifndef UDP_THREADS_SRC_THREAD_SINK_H_
#define UDP_THREADS_SRC_THREAD_SINK_H_

#include <event2/event.h>
#include "conn_center.h"
#include "mem_util.h"
#include "msg_dispatcher.h"
#include "module_loader.h"
#include "proto_msg_codec.h"
#include "scheduler.h"
#include "udp_logic_interface.h"

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

class ThreadSink : public ThreadSinkInterface
{
    CREATE_FUNC(ThreadSink)

    static void ReadCallback(evutil_socket_t sock_fd, short events, void* arg);

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
    void SetUDPThreadGroup(ThreadGroupInterface* udp_thread_group)
    {
        udp_thread_group_ = udp_thread_group;
    }

    ThreadGroupInterface* GetUDPThreadGroup()
    {
        return udp_thread_group_;
    }

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups);

    char* GetRecvBuf() const
    {
        return recv_buf_;
    }

    void OnRecvClientMsg(evutil_socket_t fd, const struct sockaddr_in* client_addr, const char* total_msg_buf,
                         size_t total_msg_len);

    ConnCenter* GetConnCenter()
    {
        return conn_center_;
    }

private:
    int BindUdpSocket();
    int LoadLocalLogic();
    int LoadLogicGroup();

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* udp_thread_group_;

    ConnCenterInterface* conn_center_;

    Scheduler scheduler_;

    int listen_sock_fd_;
    struct event* udp_event_;

    char* recv_buf_;

    ModuleLoader local_logic_loader_;
    LocalLogicInterface* local_logic_;

    LogicItemVec logic_item_vec_;

    RelatedThreadGroups* related_thread_group_;
};
}

#endif // UDP_THREADS_SRC_THREAD_SINK_H_
