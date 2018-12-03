#ifndef WORK_THREADS_SRC_WORK_THREAD_SINK_H_
#define WORK_THREADS_SRC_WORK_THREAD_SINK_H_

#include "client_center_mgr_interface.h"
#include "mem_util.h"
#include "module_loader.h"
#include "thread_center_interface.h"
#include "work_logic_interface.h"
#include "work_scheduler.h"
#include "work_threads_interface.h"

namespace work
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

class ThreadSink : public ThreadSinkInterface, public tcp::NfySinkInterface
{
    CREATE_FUNC(ThreadSink);

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

public:
    void SetThreadsCtx(const ThreadsCtx* threads_ctx)
    {
        threads_ctx_ = threads_ctx;
    }

    void SetWorkThreadGroup(ThreadGroupInterface* work_thread_group)
    {
        work_thread_group_ = work_thread_group;
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

    ThreadGroupInterface* GetWorkThreadGroup()
    {
        return work_thread_group_;
    }

    void SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group);

private:
    int LoadLocalLogic();
    int LoadLogicGroup();

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* work_thread_group_;

    tcp::ClientCenterInterface* tcp_client_center_;
    http::ClientCenterInterface* http_client_center_;
    udp::ClientCenterInterface* udp_client_center_;

    Scheduler scheduler_;

    ModuleLoader local_logic_loader_;
    LocalLogicInterface* local_logic_;

    LogicItemVec logic_item_vec_;
};
}

#endif // WORK_THREADS_SRC_WORK_THREAD_SINK_H_
