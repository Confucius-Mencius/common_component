#ifndef THREAD_CENTER_SRC_TASK_QUEUE_H_
#define THREAD_CENTER_SRC_TASK_QUEUE_H_

#include <list>
#include <mutex>
#include "thread_task.h"

class ThreadInterface;

namespace thread_center
{
class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    void SetThread(ThreadInterface* thread)
    {
        thread_ = thread;
    }

    void Release();

    int PushBack(ThreadTask* task);
    ThreadTask* PopBack();
    ThreadTask* PopFront();
    bool IsEmpty();
    int Size();
    int MaxTaskCount();

private:
    typedef std::list<ThreadTask*> TaskList;
    TaskList task_list_;

    int cur_task_count_;
    int max_task_count_;

    ThreadInterface* thread_;
};
} // namespace thread_center

#endif // THREAD_CENTER_SRC_TASK_QUEUE_H_
