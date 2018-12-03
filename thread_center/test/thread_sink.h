#ifndef THREAD_CENTER_TEST_THREAD_SINK_H_
#define THREAD_CENTER_TEST_THREAD_SINK_H_

#include "mem_util.h"
#include "thread_center_interface.h"

namespace thread_center_test
{
class ThreadSink : public ThreadSinkInterface
{
    CREATE_FUNC(ThreadSink);

public:
    ThreadSink();
    virtual ~ThreadSink();

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

private:

};
}

#endif // THREAD_CENTER_TEST_THREAD_SINK_H_
