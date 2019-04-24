#ifndef GLOBAL_THREAD_SRC_GLOBAL_THREADS_H_
#define GLOBAL_THREAD_SRC_GLOBAL_THREADS_H_

#include "global_threads_interface.h"
#include "module_loader.h"
#include "work_common_logic/global_logic_args.h"

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
    void SetRelatedThreadGroups(const work::RelatedThreadGroups* related_thread_groups) override;
    ThreadGroupInterface* GetGlobalThreadGroup() const override;
    LogicInterface* GetLogic() const override;

private:
    int LoadWorkThreads();

private:
    work::ThreadsCtx threads_ctx_;
    work::RelatedThreadGroups related_thread_groups_;

    ModuleLoader work_threads_loader_;
    work::ThreadsInterface* work_threads_;

    work::GlobalLogicArgs global_logic_args_;
};
}

#endif // GLOBAL_THREAD_SRC_GLOBAL_THREADS_H_
