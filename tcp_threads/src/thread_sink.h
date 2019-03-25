#ifndef RAW_TCP_THREADS_SRC_THREAD_SINK_H_
#define RAW_TCP_THREADS_SRC_THREAD_SINK_H_

#include "conn_mgr.h"
#include "mem_util.h"
#include "module_loader.h"
#include "new_conn.h"
#include "scheduler.h"
#include "raw_tcp_logic_interface.h"

namespace tcp
{
namespace raw
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

    void OnClientClosed(const BaseConn* conn, int task_type);
    void OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len);

private:
    int LoadCommonLogic();
    int LoadLogicGroup();
    int OnClientConnected(const NewConnCtx* new_conn_ctx);

private:
    const ThreadsCtx* threads_ctx_;
    ThreadInterface* listen_thread_;
    ThreadGroupInterface* tcp_thread_group_;
    RelatedThreadGroups* related_thread_group_;

    ModuleLoader common_logic_loader_;
    CommonLogicInterface* common_logic_;
    LogicItemVec logic_item_vec_;

    ConnMgr conn_mgr_;
    Scheduler scheduler_;
};
}
}

#endif // RAW_TCP_THREADS_SRC_THREAD_SINK_H_
