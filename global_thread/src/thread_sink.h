#ifndef GLOBAL_THREAD_SRC_THREAD_SINK_H_
#define GLOBAL_THREAD_SRC_THREAD_SINK_H_

#include "global_logic_interface.h"
#include "global_threads_interface.h"
#include "msg_dispatcher.h"
#include "mem_util.h"
#include "module_loader.h"
#include "proto_msg_codec.h"
#include "scheduler.h"
#include "thread_center_interface.h"

namespace global
{
class ThreadSink : public ThreadSinkInterface
{
    CREATE_FUNC(ThreadSink)

public:
    ThreadSink();
    virtual ~ThreadSink();

    LogicInterface* GetLogic() const
    {
        return logic_;
    }

    void SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups)
    {
        scheduler_.SetRelatedThreadGroups(related_thread_groups);
    }

    void SetReloadFinish(bool finished)
    {
        reload_finished_ = finished;
    }

    bool ReloadFinished()
    {
        return reload_finished_;
    }

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
    int LoadLogic();

private:
    const ThreadsCtx* threads_ctx_;

    ModuleLoader logic_loader_;
    LogicInterface* logic_;

    ::proto::MsgCodec msg_codec_;
    Scheduler scheduler_;
    MsgDispatcher msg_dispatcher_;

    std::atomic_bool reload_finished_;
};
}

#endif // GLOBAL_THREAD_SRC_THREAD_SINK_H_
