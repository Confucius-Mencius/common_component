#ifndef RAW_TCP_THREADS_SRC_IO_THREAD_SINK_H_
#define RAW_TCP_THREADS_SRC_IO_THREAD_SINK_H_

#include "conn_center.h"
#include "mem_util.h"
#include "msg_dispatcher.h"
#include "module_loader.h"
#include "new_conn.h"
#include "proto_msg_codec.h"
#include "raw_tcp_logic_interface.h"
#include "scheduler.h"

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
        logic = nullptr;
    }
};

typedef std::vector<LogicItem> LogicItemVec;

class IOThreadSink : public ThreadSinkInterface
{
    CREATE_FUNC(IOThreadSink)

public:
    IOThreadSink();
    virtual ~IOThreadSink();

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

    void SetIOThreadGroup(ThreadGroupInterface* io_thread_group)
    {
        io_thread_group_ = io_thread_group;
    }

    ThreadGroupInterface* GetIOThreadGroup()
    {
        return io_thread_group_;
    }

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups);

    ConnCenter* GetConnCenter()
    {
        return &conn_center_;
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
    ThreadGroupInterface* io_thread_group_;
    RelatedThreadGroups* related_thread_group_;

    ModuleLoader common_logic_loader_;
    CommonLogicInterface* common_logic_;
    LogicItemVec logic_item_vec_;

    ConnCenter conn_center_;

    ::proto::MsgCodec msg_codec_;
    Scheduler scheduler_;
    MsgDispatcher msg_dispatcher_;
};
}
}

#endif // RAW_TCP_THREADS_SRC_IO_THREAD_SINK_H_
