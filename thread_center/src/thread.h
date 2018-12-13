#ifndef THREAD_CENTER_SRC_THREAD_H_
#define THREAD_CENTER_SRC_THREAD_H_

#include <event2/event.h>
#include <event2/event_struct.h>
#include "mem_util.h"
#include "module_loader.h"
#include "task_queue.h"
#include "thread_center_interface.h"
#include "timer_axis_interface.h"

namespace thread_center
{
class ThreadGroup;

class Thread : public ThreadInterface
{
    CREATE_FUNC(Thread)

private:
    static void OnEvent(int fd, short which, void* arg);
    static void* ThreadRoutine(void* arg);

public:
    Thread();
    virtual ~Thread();

    void Release();
    int Initialize(const void* ctx);
    void Finalize();
    int Activate();
    void Freeze();

public:
    ///////////////////////// ThreadInterface /////////////////////////
    const char* GetThreadName() const override
    {
        return thread_ctx_.name.c_str();
    }

    int GetThreadIdx() const override
    {
        return thread_ctx_.idx;
    }

    pthread_t GetThreadID() const override
    {
        return thread_id_;
    }

    struct event_base* GetThreadEvBase() const override
    {
        return thread_ev_base_;
    }

    TimerAxisInterface* GetTimerAxis() const override
    {
        return timer_axis_;
    }

    bool IsStopping() const override
    {
        return stopping_;
    }

    int PushTask(Task* task) override;

public:
    // start在activate之后调用，join在freeze之前调用
    int Start();
    void Join();

    int NotifyStop();
    int NotifyReload();

    bool CanExit() const;
    int NotifyExit();

private:
    void* WorkLoop(); // 线程工作循环

    void OnStop();
    void OnReload();
    void OnExit();

    int NotifyTask(int fd);
    void OnTask();

    TaskQueue* GetTaskQueue(ThreadInterface* source_thread)
    {
        return &tq_;
    }

    int GetPipeWriteFD(ThreadInterface* source_thread)
    {
        return pipe_[1];
    }

    int LoadTimerAxis();

private:
    ThreadCtx thread_ctx_;

    pthread_t thread_id_;
    struct event_base* thread_ev_base_;

    std::mutex write_fd_mutex_;
    int pipe_[2];
    struct event* event_;
    TaskQueue tq_;

    bool stopping_;

    ModuleLoader timer_axis_loader_;
    TimerAxisInterface* timer_axis_;
};
} // namespace thread_center

#endif // THREAD_CENTER_SRC_THREAD_H_
