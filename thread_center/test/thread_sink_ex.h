#ifndef THREAD_CENTER_TEST_THREAD_SINK_EX_H_
#define THREAD_CENTER_TEST_THREAD_SINK_EX_H_

#include "mem_util.h"
#include "thread_center_interface.h"

namespace thread_center_test
{
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
    void SetReleatedThreadGroup(ThreadGroupInterface* thread_group)
    {
        thread_group_ = thread_group;
    }

private:
    ThreadGroupInterface* thread_group_; // 需要交互的线程组
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
