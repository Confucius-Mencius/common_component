/**
 * @file thread_center_interface.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef THREAD_CENTER_INC_THREAD_CENTER_INTERFACE_H_
#define THREAD_CENTER_INC_THREAD_CENTER_INTERFACE_H_

#include <pthread.h>
#include "module_interface.h"
#include "task_define.h"

/**
 * @defgroup Module_CommonComponent common component
 * @{
 */

/**
 * @defgroup Module_ThreadCenter thread center
 * @{
 */

/**
 * 起一组线程，数量为thread_count，线程函数为thread_routine，参数为thread_args。
 * 主线程可以向线程组中的某个线程推送任务，或广播，线程组中的线程可以将回复发给主线程；
 * 线程组中的线程可以向主线程推送任务，主线程可以将回复发送给线程组中的该线程
 */

class ThreadSinkInterface;
struct event_base;
class TimerAxisInterface;

typedef int ThreadType;

struct ThreadCtx
{
    const char* common_component_dir;

    // thread的属性
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

class ThreadInterface
{
public:
    virtual ~ThreadInterface()
    {
    }

    virtual const char* GetThreadName() const = 0;
    virtual int GetThreadIdx() const = 0;
    virtual pthread_t GetThreadID() const = 0;
    virtual struct event_base* GetThreadEvBase() const = 0;
    virtual TimerAxisInterface* GetTimerAxis() const = 0;
    virtual bool IsStopping() const = 0;
    virtual int PushTask(Task* task) = 0;
};

class ThreadSinkInterface
{
public:
    ThreadSinkInterface()
    {
        self_thread_ = NULL;
    }

    virtual ~ThreadSinkInterface()
    {
    }

    virtual void Release() = 0;

    virtual int OnInitialize(ThreadInterface* thread)
    {
        self_thread_ = thread;
        LOG_DEBUG(self_thread_->GetThreadName() << " " << self_thread_->GetThreadIdx() << " OnInitialize");
        return 0;
    }

    virtual void OnFinalize()
    {
        LOG_DEBUG(self_thread_->GetThreadName() << " " << self_thread_->GetThreadIdx() << " OnFinalize");
    }

    virtual int OnActivate()
    {
        LOG_DEBUG(self_thread_->GetThreadName() << " " << self_thread_->GetThreadIdx() << " OnActivate");
        return 0;
    }

    virtual void OnFreeze()
    {
        LOG_DEBUG(self_thread_->GetThreadName() << " " << self_thread_->GetThreadIdx() << " OnFreeze");
    }

    /**
     * @brief 线程启动ok后回调，一般在其中处理线程同步
     */
    virtual void OnThreadStartOk()
    {
        LOG_DEBUG(self_thread_->GetThreadName() << " " << self_thread_->GetThreadIdx() << " OnThreadStartOk");
    }

    virtual void OnStop()
    {
        LOG_DEBUG(self_thread_->GetThreadName() << " " << self_thread_->GetThreadIdx() << " OnStop");

        ////////////////////////////////////////////////////////////////////////////////
        // stop routine here...
        ////////////////////////////////////////////////////////////////////////////////
    }

    virtual void OnReload()
    {
        LOG_DEBUG(self_thread_->GetThreadName() << " " << self_thread_->GetThreadIdx() << " OnReload");
    }

    virtual void OnTask(const Task* task)
    {
        LOG_DEBUG(self_thread_->GetThreadName() << " " << self_thread_->GetThreadIdx() << " OnTask, task type: " << task->GetType());
    }

    virtual bool CanExit() const = 0;

    ThreadInterface* GetThread()
    {
        return self_thread_;
    }

protected:
    ThreadInterface* self_thread_;
};

class ThreadGroupInterface : public ModuleInterface
{
public:
    virtual ~ThreadGroupInterface()
    {
    }

    virtual ThreadInterface* CreateThread(const ThreadCtx* thread_ctx) = 0;
    virtual int GetThreadCount() const = 0;
    virtual ThreadInterface* GetThread(int thread_idx) const = 0;

    virtual int Start() = 0;
    virtual void Join() = 0;

    virtual int NotifyStop() = 0;
    virtual int NotifyReload() = 0;

    virtual bool CanExit() const = 0;
    virtual int NotifyExit() = 0;

    /**
     * @brief
     * @param task 目标线程处理完后会释放task
     * @param thread_idx 指定线程idx，=-1表示广播
     * @return
     */
    virtual int PushTaskToThread(Task* task, int thread_idx) = 0;
};

class ThreadCenterInterface : public ModuleInterface
{
public:
    virtual ~ThreadCenterInterface()
    {
    }

    virtual ThreadGroupInterface* CreateThreadGroup(const void* ctx) = 0;
};

/** @} Module_ThreadCenter */

/** @} Module_CommonComponent */

#endif // THREAD_CENTER_INC_THREAD_CENTER_INTERFACE_H_
