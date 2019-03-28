#ifndef WORK_THREADS_SRC_THREAD_SINK_H_
#define WORK_THREADS_SRC_THREAD_SINK_H_

#include "mem_util.h"
#include "module_loader.h"
#include "msg_dispatcher.h"
#include "proto_msg_codec.h"
#include "scheduler.h"
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

class ThreadSink : public ThreadSinkInterface
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

private:
    int LoadCommonLogic();
    int LoadLogicGroup();

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* work_thread_group_;

    ModuleLoader common_logic_loader_;
    CommonLogicInterface* common_logic_;
    LogicItemVec logic_item_vec_;

    ::proto::MsgCodec msg_codec_;
    Scheduler scheduler_;
    MsgDispatcher msg_dispatcher_;
};
}

#endif // WORK_THREADS_SRC_THREAD_SINK_H_
