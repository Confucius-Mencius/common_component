#ifndef WORK_THREADS_SRC_WORK_THREADS_H_
#define WORK_THREADS_SRC_WORK_THREADS_H_

#include "module_loader.h"
#include "thread_sink.h"
#include "work_threads_interface.h"

namespace work
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
    int CreateThreadGroup(const char* name_prefix) override;
    void SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_group) override;
    ThreadGroupInterface* GetWorkThreadGroup() const override;

private:
    ThreadsCtx threads_ctx_;
    RelatedThreadGroups related_thread_groups_;
    ThreadGroupInterface* work_thread_group_;
};
}

#endif // WORK_THREADS_SRC_WORK_THREADS_H_
