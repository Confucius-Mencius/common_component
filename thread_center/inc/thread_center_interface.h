/**
 * @file thread_center_interface.h
 * @brief
 * @author BrentHuang (guang11cheng@qq.com)
 * @copyright Copyright (C) 2008-2016, MOON Corporation. All rights reserved.
 */

#ifndef THREAD_CENTER_INC_THREAD_CENTER_INTERFACE_H_
#define THREAD_CENTER_INC_THREAD_CENTER_INTERFACE_H_

#include <iomanip>
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
class TimeServiceInterface;
class RandomEngineInterface;
class TransCenterInterface;
class ConnCenterMgrInterface;
class ClientCenterMgrInterface;

enum ThreadType
{
    THREAD_TYPE_MIN = 0,
    THREAD_TYPE_GLOBAL = THREAD_TYPE_MIN,
    THREAD_TYPE_WORK,
    THREAD_TYPE_BURDEN,
    THREAD_TYPE_TCP_LISTEN,
    THREAD_TYPE_TCP,
    THREAD_TYPE_HTTP,
    THREAD_TYPE_UDP,
    THREAD_TYPE_MAX
};

namespace base
{
class MsgDispatcherInterface;
}

namespace base
{
struct AsyncParams;
}

struct ThreadCtx
{
    const char* common_component_dir;
    int need_reply_msg_check_interval;

    // thread的属性
    std::string name;
    int idx;
    ThreadSinkInterface* sink;

    ThreadCtx() : name("")
    {
        common_component_dir = NULL;
        need_reply_msg_check_interval = 0;
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
    virtual TimeServiceInterface* GetTimeService() const = 0;
    virtual RandomEngineInterface* GetRandomEngine() = 0;
    virtual TransCenterInterface* GetTransCenter() const = 0;
    virtual ConnCenterMgrInterface* GetConnCenterMgr() const = 0;
    virtual ClientCenterMgrInterface* GetClientCenterMgr() const = 0;
    virtual base::MsgDispatcherInterface* GetMsgDispatcher() const = 0;

    virtual bool IsStopping() const = 0;

    virtual int PushTask(Task* task) = 0;

    /**
     *
     * @param task
     * @param params 为NULL时接口返回OK_TRANS_ID表示成功，返回FAILED_TRANS_ID表示失败
     * @param target_thread
     * @return
     */
//    virtual TransId ScheduleTask(Task* task, const base::AsyncParams* params, ThreadInterface* target_thread) = 0;
};

class ThreadSinkInterface
{
public:
    ThreadSinkInterface()
    {
        thread_ = NULL;
    }

    virtual ~ThreadSinkInterface()
    {
    }

    virtual void Release() = 0;

    virtual int OnInitialize(ThreadInterface* thread)
    {
        thread_ = thread;
        LOG_TRACE(thread_->GetThreadName()
                  << " initializing..."); // TODO 把这几个函数里的LOG_TRACE改为LOG_INFO，但是LOG_INFO会报错：invalid operands of types ‘const char*’ and ‘const char [23]’ to binary ‘operator<<’
        return 0;
    }

    virtual void OnFinalize()
    {
        LOG_TRACE(thread_->GetThreadName() << " finalizing...");
    }

    virtual int OnActivate()
    {
        LOG_TRACE(thread_->GetThreadName() << " activating...");
        return 0;
    }

    virtual void OnFreeze()
    {
        LOG_TRACE(thread_->GetThreadName() << " freezing...");
    }

    /**
     * @brief 线程启动ok后回调，一般在其中处理线程同步
     */
    virtual void OnThreadStartOk()
    {
        LOG_TRACE(thread_->GetThreadName() << " start ok. thread id: " << setiosflags(std::ios::showbase) << std::hex
                  << thread_->GetThreadID());
    }

    virtual void OnStop()
    {
        LOG_TRACE(thread_->GetThreadName() << " stopping...");

        ////////////////////////////////////////////////////////////////////////////////
        // stop routine here...
        ////////////////////////////////////////////////////////////////////////////////
    }

    virtual void OnReload()
    {
        LOG_TRACE(thread_->GetThreadName() << " reloading...");
    }

    virtual void OnTask(const Task* task)
    {
        LOG_TRACE(thread_->GetThreadName() << " on task, type: " << task->GetCtx()->task_type);
    }

    virtual bool CanExit() const = 0;

    ThreadInterface* GetThread()
    {
        return thread_;
    }

protected:
    ThreadInterface* thread_;
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

    virtual int NotifyStop() = 0;
    virtual int NotifyReload() = 0;

    virtual bool CanExit() const = 0;
    virtual int NotifyExit() = 0;

    /**
     * @brief
     * @param task
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

    virtual ThreadGroupInterface* CreateThreadGroup() = 0;
};

/** @} Module_ThreadCenter */

/** @} Module_CommonComponent */

#endif // THREAD_CENTER_INC_THREAD_CENTER_INTERFACE_H_
