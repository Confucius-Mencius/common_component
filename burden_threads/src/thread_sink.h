#ifndef BURDEN_THREADS_SRC_THREAD_SINK_H_
#define BURDEN_THREADS_SRC_THREAD_SINK_H_

#include "burden_logic_interface.h"
#include "burden_threads_interface.h"
#include "msg_dispatcher.h"
#include "mem_util.h"
#include "module_loader.h"
#include "proto_msg_codec.h"
#include "scheduler.h"

namespace burden
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

public:
    ThreadSink();
    virtual ~ThreadSink();

    void SetBurdenThreadGroup(ThreadGroupInterface* burden_thread_group)
    {
        burden_thread_group_ = burden_thread_group;
    }

    ThreadGroupInterface* GetBurdenThreadGroup()
    {
        return burden_thread_group_;
    }

    void SetRelatedThreadGroup(RelatedThreadGroups* related_thread_groups);

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

private:
    int LoadCommonLogic();
    int LoadLogicGroup();

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* burden_thread_group_;

    ModuleLoader common_logic_loader_;
    CommonLogicInterface* common_logic_;
    LogicItemVec logic_item_vec_;

    ::proto::MsgCodec msg_codec_;
    Scheduler scheduler_;
    MsgDispatcher msg_dispatcher_;
};
}

#endif // BURDEN_THREADS_SRC_THREAD_SINK_H_
