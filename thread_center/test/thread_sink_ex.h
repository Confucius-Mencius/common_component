#ifndef THREAD_CENTER_TEST_THREAD_SINK_EX_H_
#define THREAD_CENTER_TEST_THREAD_SINK_EX_H_

#include "mem_util.h"
#include "thread_center_interface.h"

namespace thread_center_test
{
struct UpstreamThreadSinkCtx
{
    ThreadGroupInterface* thread_group; // 需要交互的线程组
};

class UpstreamThreadSink : public ThreadSinkInterface
{
    CREATE_FUNC(UpstreamThreadSink);

public:
    UpstreamThreadSink();
    virtual ~UpstreamThreadSink();

    ///////////////////////// ThreadSinkInterface /////////////////////////
    void Release() override;
    virtual int OnInitialize(ThreadInterface* thread);
    void OnFinalize() override;
    int OnActivate() override;
    void OnFreeze() override;
    void OnThreadStartOk() override;
    void OnStop() override;
    void OnReload() override;
    void OnTask(const Task* task) override;
    bool CanExit() const override;

public:
    void SetSinkCtx(const UpstreamThreadSinkCtx& ctx)
    {
        sink_ctx_ = ctx;
    }

private:
    UpstreamThreadSinkCtx sink_ctx_;
};

class ThreadSinkEx : public ThreadSinkInterface
{
    CREATE_FUNC(ThreadSinkEx);

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

private:

};
}

#endif // THREAD_CENTER_TEST_THREAD_SINK_EX_H_
