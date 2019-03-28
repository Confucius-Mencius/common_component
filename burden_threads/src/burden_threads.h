#ifndef BURDEN_THREADS_SRC_BURDEN_THREADS_H_
#define BURDEN_THREADS_SRC_BURDEN_THREADS_H_

#include "burden_threads_interface.h"
#include "module_loader.h"
#include "thread_sink.h"

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
    void SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_groups) override;
    ThreadGroupInterface* GetBurdenThreadGroup() const override;

private:
    ThreadsCtx threads_ctx_;
    RelatedThreadGroups related_thread_groups_;
    ThreadGroupInterface* burden_thread_group_;

    typedef std::vector<ThreadSink*> ThreadSinkVec;
    ThreadSinkVec thread_sink_vec_;
};
}

#endif // BURDEN_THREADS_SRC_BURDEN_THREADS_H_
