#ifndef GLOBAL_THREAD_SRC_GLOBAL_THREADS_H_
#define GLOBAL_THREAD_SRC_GLOBAL_THREADS_H_

#include "global_threads_interface.h"
#include "module_loader.h"
#include "thread_sink.h"

namespace global
{
class Threads : public ThreadsInterface
{
public:
    Threads();
    virtual ~Threads();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ThreadsInterface /////////////////////////
    int CreateThreadGroup() override;
    void SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_groups) override;
    ThreadGroupInterface* GetGlobalThreadGroup() const override;
    LogicInterface* GetLogic() const override;
    void SetReloadFinish(bool finished) override;
    bool ReloadFinished() override;

private:
    int CreateGlobalThread();

private:
    ThreadsCtx threads_ctx_;
    ThreadGroupInterface* global_thread_group_;
    ThreadSink* global_thread_sink_;
    RelatedThreadGroups related_thread_groups_;
};
}

#endif // GLOBAL_THREAD_SRC_GLOBAL_THREADS_H_
