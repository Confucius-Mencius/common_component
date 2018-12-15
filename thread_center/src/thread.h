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

struct ThreadCtx
{
    const char* common_component_dir;
    std::string name;
    int idx;
    ThreadSinkInterface* sink;

    ThreadCtx() : name("")
    {
        common_component_dir = NULL;
        idx = -1;
        sink = NULL;
    }
};

class Thread : public ThreadInterface, public TimerSinkInterface
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

    ThreadSinkInterface* GetThreadSink() override
    {
        return thread_ctx_.sink;
    }

    bool IsStopping() const override
    {
        return stopping_;
    }

    void PushTask(Task* task) override;

    ///////////////////////// TimerSinkInterface /////////////////////////
    void OnTimer(TimerID timer_id, void* data, size_t len, int times) override;

public:
    // start在activate之后调用，join在freeze之前调用
    int Start();
    void Join();

    void NotifyStop();
    void NotifyReload();

    bool CanExit() const;
    void NotifyExit();

private:
    void* WorkLoop(); // 线程工作循环

    void OnStop();
    void OnReload();
    void OnExit();

    void NotifyTask(int fd);
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

    void StartPendingNotifyTimer();
    void StopPendingNotifyTimer();

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

    typedef std::list<char> PendingNotifyList;
    PendingNotifyList pending_notify_list_;
};
} // namespace thread_center

#endif // THREAD_CENTER_SRC_THREAD_H_
