#include "task_queue.h"
#include "container_util.h"
#include "thread.h"

namespace thread_center
{
TaskQueue::TaskQueue() : task_list_()
{
    cur_task_count_ = 0;
    max_task_count_ = 0;
    thread_ = NULL;
}

TaskQueue::~TaskQueue()
{
}

void TaskQueue::Release()
{
    RELEASE_CONTAINER(task_list_);
}

int TaskQueue::PushBack(ThreadTask* task)
{
    task_list_.push_back(task);
    ++cur_task_count_;

    if (cur_task_count_ > max_task_count_)
    {
        max_task_count_ = cur_task_count_;
        LOG_WARN(thread_->GetThreadName() << " max task count: " << max_task_count_);
    }

    return 0;
}

ThreadTask* TaskQueue::PopBack()
{
    if (task_list_.empty())
    {
        return NULL;
    }

    ThreadTask* task = task_list_.back();
    task_list_.pop_back();
    --cur_task_count_;

    return task;
}

ThreadTask* TaskQueue::PopFront()
{
    if (task_list_.empty())
    {
        return NULL;
    }

    ThreadTask* task = task_list_.front();
    task_list_.pop_front();
    --cur_task_count_;
    LOG_DEBUG(thread_->GetThreadName() << " task count in queue: " << cur_task_count_);

    return task;
}

bool TaskQueue::IsEmpty()
{
    return (0 == cur_task_count_);
}

int TaskQueue::Size()
{
    return cur_task_count_;
}

int TaskQueue::MaxTaskCount()
{
    return max_task_count_;
}
} // namespace thread_center
