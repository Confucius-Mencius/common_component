#include "thread_sink.h"
#include "task_count.h"

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
    delete this;
}

int ThreadSink::OnInitialize(ThreadInterface* thread)
{
    if (ThreadSinkInterface::OnInitialize(thread) != 0)
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

void ThreadSink::OnThreadStartOk()
{
    ThreadSinkInterface::OnThreadStartOk();
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

void ThreadSink::OnTask(const Task* task)
{
    ThreadSinkInterface::OnTask(task);
    g_task_count--;
}

bool ThreadSink::CanExit() const
{
    return true;
}
}
