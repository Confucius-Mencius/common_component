#include "thread_sink_ex.h"

namespace thread_center_test
{
UpstreamThreadSink::UpstreamThreadSink()
{

}

UpstreamThreadSink::~UpstreamThreadSink()
{

}

void UpstreamThreadSink::Release()
{
    delete this;
}

int UpstreamThreadSink::OnInitialize(ThreadInterface* thread)
{
    if (ThreadSinkInterface::OnInitialize(thread) != 0)
    {
        return -1;
    }

    return 0;
}

void UpstreamThreadSink::OnFinalize()
{
    ThreadSinkInterface::OnFinalize();
}

int UpstreamThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    return 0;
}

void UpstreamThreadSink::OnFreeze()
{
    ThreadSinkInterface::OnFreeze();
}

void UpstreamThreadSink::OnThreadStartOk()
{
    ThreadSinkInterface::OnThreadStartOk();
}

void UpstreamThreadSink::OnStop()
{
    ThreadSinkInterface::OnStop();

    ////////////////////////////////////////////////////////////////////////////////
    // stop routine...
    ////////////////////////////////////////////////////////////////////////////////
}

void UpstreamThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    TaskCtx task_ctx;
    task_ctx.source_thread = thread_;

    Task* task = Task::Create(&task_ctx);
    if (NULL == task)
    {
        return;
    }

    sink_ctx_.thread_group->PushTaskToThread(task, 5);
}

void UpstreamThreadSink::OnTask(const Task* task)
{
    ThreadSinkInterface::OnTask(task);
}

bool UpstreamThreadSink::CanExit() const
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

    TaskCtx new_task_ctx;
    new_task_ctx.source_thread = thread_;

    Task* new_task = Task::Create(&new_task_ctx);
    if (NULL == new_task)
    {
        return;
    }

    task->GetCtx()->source_thread->PushTask(new_task);
}

bool ThreadSinkEx::CanExit() const
{
    return true;
}
}
