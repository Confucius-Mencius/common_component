#ifndef THREAD_CENTER_SRC_THREAD_H_
#define THREAD_CENTER_SRC_THREAD_H_

#include <event2/event.h>
#include <event2/event_struct.h>
#include "client_center_mgr_interface.h"
#include "conn_center_mgr_interface.h"
#include "mem_util.h"
#include "module_loader.h"
#include "msg_dispatcher.h"
#include "random_engine_interface.h"
#include "task_queue.h"
#include "thread_center_interface.h"
#include "time_service_interface.h"
#include "timer_axis_interface.h"
#include "trans_center_interface.h"

namespace thread_center
{
class ThreadGroup;

class Thread : public ThreadInterface
{
    CREATE_FUNC(Thread);

public:
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

    TimeServiceInterface* GetTimeService() const override
    {
        return time_service_;
    }

    RandomEngineInterface* GetRandomEngine() override
    {
        return random_engine_;
    }

    TransCenterInterface* GetTransCenter() const override
    {
        return trans_center_;
    }

    ConnCenterMgrInterface* GetConnCenterMgr() const override
    {
        return conn_center_mgr_;
    }

    ClientCenterMgrInterface* GetClientCenterMgr() const override
    {
        return client_center_mgr_;
    }

    base::MsgDispatcherInterface* GetMsgDispatcher() const override
    {
        return const_cast<MsgDispatcher*>(&msg_dispatcher_);
    }

    bool IsStopping() const override
    {
        return stopping_;
    }

    int PushTask(Task* task) override;

//    TransId ScheduleTask(Task* task, const base::AsyncParams* params, ThreadInterface* target_thread) override;

public:
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

    TaskQueue* GetTaskQueue(ThreadInterface* upstream_thread)
    {
        return &tq_;
    }

    int GetPipeWriteFD(ThreadInterface* upstream_thread)
    {
        return pipe_[1];
    }

    int LoadTimerAxis();
    int LoadTimeService();
    int LoadRandomEngine();
    int LoadTransCenter();
    int LoadConnCenterMgr();
    int LoadClientCenterMgr();

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

    ModuleLoader time_service_loader_;
    TimeServiceInterface* time_service_;

    ModuleLoader random_engine_loader_;
    RandomEngineInterface* random_engine_;

    ModuleLoader trans_center_loader_;
    TransCenterInterface* trans_center_;

    ModuleLoader conn_center_mgr_loader_;
    ConnCenterMgrInterface* conn_center_mgr_;

    ModuleLoader client_center_mgr_loader_;
    ClientCenterMgrInterface* client_center_mgr_;

    MsgDispatcher msg_dispatcher_;
    MsgDispatcher normal_msg_dispatcher_;
};
} // namespace thread_center

#endif // THREAD_CENTER_SRC_THREAD_H_
