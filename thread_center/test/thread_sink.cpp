#include "thread_sink.h"
#include <unistd.h>
#include "thread_task_count.h"

namespace thread_center_test
{
ThreadSink::ThreadSink()
{
}

ThreadSink::~ThreadSink()
{
}

void ThreadSink::Release()
{
    if (self_thread_ != nullptr)
    {
        LOG_TRACE(self_thread_->GetThreadName() << " Release");
    }

    delete this;
}

int ThreadSink::OnInitialize(ThreadInterface* thread, const void* ctx)
{
    if (ThreadSinkInterface::OnInitialize(thread, ctx) != 0)
    {
        return -1;
    }

    return 0;
}

void ThreadSink::OnFinalize()
{
    ThreadSinkInterface::OnFinalize();
}

int ThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    return 0;
}

void ThreadSink::OnFreeze()
{
    ThreadSinkInterface::OnFreeze();
}

void ThreadSink::OnThreadStartOK()
{
    ThreadSinkInterface::OnThreadStartOK();
}

void ThreadSink::OnStop()
{
    ThreadSinkInterface::OnStop();

    ////////////////////////////////////////////////////////////////////////////////
    // stop routine...
    ////////////////////////////////////////////////////////////////////////////////
}

void ThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();
}

void ThreadSink::OnTask(const ThreadTask* task)
{
    ThreadSinkInterface::OnTask(task);
    // TODO
    // usleep(rand() % 5000); // 微秒。模拟程序逻辑执行时间
    g_thread_task_count--;
}

bool ThreadSink::CanExit() const
{
    return true;
}
}
