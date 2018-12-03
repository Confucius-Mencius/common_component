#ifndef THREAD_CENTER_SRC_TASK_QUEUE_H_
#define THREAD_CENTER_SRC_TASK_QUEUE_H_

#include <list>
#include <mutex>
#include "task_define.h"

namespace thread_center
{
class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    void Release();

    int PushBack(Task* task);
    Task* PopBack();
    Task* PopFront();
    bool IsEmpty();
    int Size();
    int MaxTaskCount();

private:
    std::mutex mutex_;

    typedef std::list<Task*> TaskList;
    TaskList task_list_;

    int cur_task_count_;
    int max_task_count_;
};
} // namespace thread_center

#endif // THREAD_CENTER_SRC_TASK_QUEUE_H_
