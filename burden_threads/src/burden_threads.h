#ifndef BURDEN_THREADS_SRC_BURDEN_THREADS_H_
#define BURDEN_THREADS_SRC_BURDEN_THREADS_H_

#include "burden_thread_sink.h"
#include "burden_threads_interface.h"
#include "module_loader.h"

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
    ThreadGroupInterface* GetBurdenThreadGroup() const override;
    void SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group) override;

private:
    int CreateBurdenThreads();

private:
    ThreadsCtx threads_ctx_;
    ThreadGroupInterface* burden_thread_group_;

    typedef std::vector<ThreadInterface*> BurdenThreadVec;
    BurdenThreadVec burden_thread_vec_;

    typedef std::vector<ThreadSink*> BurdenThreadSinkVec;
    BurdenThreadSinkVec burden_thread_sink_vec_;

    RelatedThreadGroup related_thread_group_;
};
}

#endif // BURDEN_THREADS_SRC_BURDEN_THREADS_H_
