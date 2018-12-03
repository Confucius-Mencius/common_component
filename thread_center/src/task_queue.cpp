#include "task_queue.h"
#include "container_util.h"

namespace thread_center
{
TaskQueue::TaskQueue() : mutex_(), task_list_()
{
    cur_task_count_ = 0;
    max_task_count_ = 0;
}

TaskQueue::~TaskQueue()
{
}

void TaskQueue::Release()
{
    std::lock_guard<std::mutex> lock(mutex_);
    RELEASE_CONTAINER(task_list_);
}

int TaskQueue::PushBack(Task* task)
{
    std::lock_guard<std::mutex> lock(mutex_);

    task_list_.push_back(task);
    ++cur_task_count_;

    if (cur_task_count_ > max_task_count_)
    {
        max_task_count_ = cur_task_count_;
    }

    return 0;
}

Task* TaskQueue::PopBack()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (task_list_.empty())
    {
        return NULL;
    }

    Task* task = task_list_.back();
    task_list_.pop_back();
    --cur_task_count_;

    return task;
}

Task* TaskQueue::PopFront()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (task_list_.empty())
    {
        return NULL;
    }

    Task* task = task_list_.front();
    task_list_.pop_front();
    --cur_task_count_;

    return task;
}

bool TaskQueue::IsEmpty()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return (0 == cur_task_count_);
}

int TaskQueue::Size()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return cur_task_count_;
}

int TaskQueue::MaxTaskCount()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return max_task_count_;
}
} // namespace thread_center
