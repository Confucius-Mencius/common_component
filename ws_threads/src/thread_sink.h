#ifndef WS_THREADS_SRC_THREAD_SINK_H_
#define WS_THREADS_SRC_THREAD_SINK_H_

#include "conn_mgr.h"
#include "mem_util.h"
#include "module_loader.h"
#include "new_conn.h"
#include "ws_scheduler.h"
#include "ws_logic_interface.h"

namespace ws
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
    void SetWSThreadGroup(ThreadGroupInterface* ws_thread_group)
    {
        ws_thread_group_ = ws_thread_group;
    }

    ThreadGroupInterface* GetWSThreadGroup()
    {
        return ws_thread_group_;
    }

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups);

    ConnMgr* GetConnMgr()
    {
        return &conn_mgr_;
    }

    int OnClientConnected(const NewConnCtx* new_conn_ctx);
    void OnClientClosed(const BaseConn* conn);
    void OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len);

    void OnGet(const ConnGUID* conn_guid);
    void OnPost(const ConnGUID* conn_guid, const void* data, size_t len);

private:
    int LoadCommonLogic();
    int LoadLogicGroup();

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* ws_thread_group_;
    RelatedThreadGroups* related_thread_group_;

    ModuleLoader ws_common_logic_loader_;
    CommonLogicInterface* ws_common_logic_;
    LogicItemVec ws_logic_item_vec_;

    ConnMgr conn_mgr_;
    Scheduler ws_scheduler_;
};
}

#endif // WS_THREADS_SRC_THREAD_SINK_H_
