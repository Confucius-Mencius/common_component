#ifndef BURDEN_THREADS_SRC_BURDEN_THREADS_H_
#define BURDEN_THREADS_SRC_BURDEN_THREADS_H_

#include "burden_threads_interface.h"
#include "module_loader.h"
#include "work_logic/burden_logic_args.h"

namespace burden
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
    ThreadGroupInterface* GetBurdenThreadGroup() const override;

private:
    int LoadWorkThreads();

private:
    work::ThreadsCtx threads_ctx_;
    work::RelatedThreadGroups related_thread_groups_;

    ModuleLoader work_threads_loader_;
    work::ThreadsInterface* work_threads_;

    work::BurdenLogicArgs burden_logic_args_;
};
}

#endif // BURDEN_THREADS_SRC_BURDEN_THREADS_H_
