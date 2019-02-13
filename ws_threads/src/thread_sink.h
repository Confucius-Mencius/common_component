#ifndef WS_THREADS_SRC_THREAD_SINK_H_
#define WS_THREADS_SRC_THREAD_SINK_H_

#include <set>
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event_struct.h>
#include <event2/util.h>
#include <libwebsockets.h>
#include "mem_util.h"
#include "module_loader.h"
#include "conn_mgr.h"
#include "ws_logic_interface.h"
#include "scheduler.h"

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

    void OnClientClosed(const BaseConn* conn);
    void OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len);

private:
    int BindWSSocket();
    int LoadCommonLogic();
    int LoadLogicGroup();
//    void OnClientConnected(const NewConnCtx* new_conn_ctx);

private:
    const ThreadsCtx* threads_ctx_;
    ThreadInterface* listen_thread_;
    ThreadGroupInterface* ws_thread_group_;
    RelatedThreadGroups* related_thread_group_;

    struct lws_context_creation_info ws_info_;
    char ws_iface_[128];
//    struct lws_http_mount http_mount_;
    char cert_path[1024];
    char key_path[1024];
    void* ws_foreign_loops_[1];
    struct lws_context* ws_context_;

    ModuleLoader common_logic_loader_;
    CommonLogicInterface* common_logic_;
    LogicItemVec logic_item_vec_;

    ConnMgr conn_mgr_;
    Scheduler scheduler_;
};
}

#endif // WS_THREADS_SRC_THREAD_SINK_H_
