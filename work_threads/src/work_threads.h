#ifndef WORK_THREADS_SRC_WORK_THREADS_H_
#define WORK_THREADS_SRC_WORK_THREADS_H_

#include "module_loader.h"
#include "work_thread_sink.h"
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
    int CreateThreadGroup() override;
    ThreadGroupInterface* GetWorkThreadGroup() const override;
    void SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group) override;

private:
    int CreateWorkThreads();

private:
    ThreadsCtx threads_ctx_;
    ThreadGroupInterface* work_thread_group_;

    typedef std::vector<ThreadInterface*> WorkThreadVec;
    WorkThreadVec work_thread_vec_;

    typedef std::vector<ThreadSink*> WorkThreadSinkVec;
    WorkThreadSinkVec work_thread_sink_vec_;

    RelatedThreadGroup related_thread_group_;
};
}

#endif // WORK_THREADS_SRC_WORK_THREADS_H_
