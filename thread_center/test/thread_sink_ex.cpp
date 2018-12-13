#include "thread_sink_ex.h"

namespace thread_center_test
{
SourceThreadSink::SourceThreadSink()
{
}

SourceThreadSink::~SourceThreadSink()
{
}

void SourceThreadSink::Release()
{
    delete this;
}

int SourceThreadSink::OnInitialize(ThreadInterface* thread)
{
    if (ThreadSinkInterface::OnInitialize(thread) != 0)
    {
        return -1;
    }

    return 0;
}

void SourceThreadSink::OnFinalize()
{
    ThreadSinkInterface::OnFinalize();
}

int SourceThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    return 0;
}

void SourceThreadSink::OnFreeze()
{
    ThreadSinkInterface::OnFreeze();
}

void SourceThreadSink::OnThreadStartOk()
{
    ThreadSinkInterface::OnThreadStartOk();
}

void SourceThreadSink::OnStop()
{
    ThreadSinkInterface::OnStop();

    ////////////////////////////////////////////////////////////////////////////////
    // stop routine...
    ////////////////////////////////////////////////////////////////////////////////
}

void SourceThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    Task* task = new Task(-1, self_thread_, NULL, NULL, 0);
    if (NULL == task)
    {
        LOG_ERROR("failed to create task");
        return;
    }

    sink_ctx_.thread_group->PushTaskToThread(task, 5);
}

void SourceThreadSink::OnTask(const Task* task)
{
    ThreadSinkInterface::OnTask(task);
}

bool SourceThreadSink::CanExit() const
{
    return true;
}

ThreadSinkEx::ThreadSinkEx()
{
}

ThreadSinkEx::~ThreadSinkEx()
{
}

void ThreadSinkEx::Release()
{
    delete this;
}

int ThreadSinkEx::OnInitialize(ThreadInterface* thread)
{
    if (ThreadSinkInterface::OnInitialize(thread) != 0)
    {
        return -1;
    }

    return 0;
}

void ThreadSinkEx::OnFinalize()
{
    ThreadSinkInterface::OnFinalize();
}

int ThreadSinkEx::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    return 0;
}

void ThreadSinkEx::OnFreeze()
{
    ThreadSinkInterface::OnFreeze();
}

void ThreadSinkEx::OnStop()
{
    ThreadSinkInterface::OnStop();

    ////////////////////////////////////////////////////////////////////////////////
    // stop routine...
    ////////////////////////////////////////////////////////////////////////////////
}

void ThreadSinkEx::OnReload()
{
    ThreadSinkInterface::OnReload();
}

void ThreadSinkEx::OnTask(const Task* task)
{
    ThreadSinkInterface::OnTask(task);

    Task* new_task = new Task(-1, self_thread_, NULL, NULL, 0);
    if (NULL == new_task)
    {
        LOG_ERROR("failed to create task");
        return;
    }

    task->GetSourceThread()->PushTask(new_task);
}

bool ThreadSinkEx::CanExit() const
{
    return true;
}
}
