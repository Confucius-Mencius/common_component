#ifndef THREAD_CENTER_TEST_THREAD_SINK_EX_H_
#define THREAD_CENTER_TEST_THREAD_SINK_EX_H_

#include "mem_util.h"
#include "thread_center_interface.h"

namespace thread_center_test
{
struct SourceThreadSinkCtx
{
    ThreadGroupInterface* thread_group; // 需要交互的线程组
};

class SourceThreadSink : public ThreadSinkInterface
{
    CREATE_FUNC(SourceThreadSink)

public:
    SourceThreadSink();
    virtual ~SourceThreadSink();

    ///////////////////////// ThreadSinkInterface /////////////////////////
    void Release() override;
    virtual int OnInitialize(ThreadInterface* thread);
    void OnFinalize() override;
    int OnActivate() override;
    void OnFreeze() override;
    void OnThreadStartOK() override;
    void OnStop() override;
    void OnReload() override;
    void OnTask(const Task* task) override;
    bool CanExit() const override;

public:
    void SetSinkCtx(const SourceThreadSinkCtx& ctx)
    {
        sink_ctx_ = ctx;
    }

private:
    SourceThreadSinkCtx sink_ctx_;
};

class ThreadSinkEx : public ThreadSinkInterface
{
    CREATE_FUNC(ThreadSinkEx)

public:
    ThreadSinkEx();
    virtual ~ThreadSinkEx();

    ///////////////////////// ThreadSinkInterface /////////////////////////
    void Release() override;
    virtual int OnInitialize(ThreadInterface* thread);
    void OnFinalize() override;
    int OnActivate() override;
    void OnFreeze() override;
    virtual void OnStop();
    virtual void OnReload();
    virtual void OnTask(const Task* task);
    bool CanExit() const override;
};
}

#endif // THREAD_CENTER_TEST_THREAD_SINK_EX_H_
