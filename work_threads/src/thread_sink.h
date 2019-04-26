#ifndef WORK_THREADS_SRC_THREAD_SINK_H_
#define WORK_THREADS_SRC_THREAD_SINK_H_

#include "client_center_mgr_interface.h"
#include "http_client_center_interface.h"
#include "mem_util.h"
#include "module_loader.h"
#include "msg_dispatcher.h"
#include "proto_msg_codec.h"
#include "proto_tcp_client_center_interface.h"
#include "scheduler.h"
#include "trans_center_interface.h"
#include "work_logic_interface.h"
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
        logic = nullptr;
    }
};

typedef std::vector<LogicItem> LogicItemVec;

class ThreadSink : public ThreadSinkInterface, public tcp::proto::NfySinkInterface
{
    CREATE_FUNC(ThreadSink)

public:
    ThreadSink();
    virtual ~ThreadSink();

    void SetWorkThreadGroup(ThreadGroupInterface* work_thread_group)
    {
        work_thread_group_ = work_thread_group;
    }

    ThreadGroupInterface* GetWorkThreadGroup()
    {
        return work_thread_group_;
    }

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups);

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

    ///////////////////////// ThreadSinkInterface /////////////////////////
    void OnRecvNfy(const Peer& peer, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len) override;

    TransCenterInterface* GetTransCenter() const
    {
        return trans_center_;
    }

    tcp::proto::ClientCenterInterface* GetProtoTCPClientCenter()
    {
        return proto_tcp_client_center_;
    }

    http::ClientCenterInterface* GetHTTPClientCenter()
    {
        return http_client_center_;
    }

private:
    int LoadCommonLogic();
    int LoadLogicGroup();

    int LoadTransCenter();
    int LoadClientCenterMgr();

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* work_thread_group_;

    ModuleLoader common_logic_loader_;
    CommonLogicInterface* common_logic_;
    LogicItemVec logic_item_vec_;

    ::proto::MsgCodec msg_codec_;
    Scheduler scheduler_;
    MsgDispatcher msg_dispatcher_;

    ModuleLoader trans_center_loader_;
    TransCenterInterface* trans_center_;

    ModuleLoader client_center_mgr_loader_;
    ClientCenterMgrInterface* client_center_mgr_;

    tcp::proto::ClientCenterInterface* proto_tcp_client_center_;
    http::ClientCenterInterface* http_client_center_;
};
}

#endif // WORK_THREADS_SRC_THREAD_SINK_H_
