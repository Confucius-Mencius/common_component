#ifndef GLOBAL_THREAD_SRC_GLOBAL_THREAD_SINK_H_
#define GLOBAL_THREAD_SRC_GLOBAL_THREAD_SINK_H_

#include <mutex>
#include "client_center_mgr_interface.h"
#include "global_logic_interface.h"
#include "global_scheduler.h"
#include "global_threads_interface.h"
#include "mem_util.h"
#include "module_loader.h"
#include "thread_center_interface.h"

namespace global
{
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
    void OnThreadStartOK() override;
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

    LogicInterface* GetLogic() const
    {
        return logic_;
    }

    void SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group)
    {
        scheduler_.SetRelatedThreadGroup(related_thread_group);
    }

    void SetReloadFinish(bool finish)
    {
        std::lock_guard<std::mutex> lock(reload_finished_mutex_);
        reload_finished_ = finish;
    }

    bool ReloadFinished()
    {
        std::lock_guard<std::mutex> lock(reload_finished_mutex_);
        return reload_finished_;
    }

private:
    int LoadLogic();

private:
    const ThreadsCtx* threads_ctx_;

    tcp::ClientCenterInterface* tcp_client_center_;
    http::ClientCenterInterface* http_client_center_;
    udp::ClientCenterInterface* udp_client_center_;

    Scheduler scheduler_;

    ModuleLoader logic_loader_;
    LogicInterface* logic_;

    std::mutex reload_finished_mutex_;
    bool reload_finished_;
};
}

#endif // GLOBAL_THREAD_SRC_GLOBAL_THREAD_SINK_H_
