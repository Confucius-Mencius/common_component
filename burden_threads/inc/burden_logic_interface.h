#ifndef BURDEN_THREADS_INC_BURDEN_LOGIC_INTERFACE_H_
#define BURDEN_THREADS_INC_BURDEN_LOGIC_INTERFACE_H_

#include <stddef.h>
#include "module_interface.h"

class ConfCenterInterface;
class TimerAxisInterface;
class TimeServiceInterface;
class RandomEngineInterface;

namespace base
{
class MsgDispatcherInterface;
}

struct ConnGuid;

namespace global
{
class LogicInterface;
}

namespace burden
{
class ClientMgrInterface;
class SchedulerInterface;
class LocalLogicInterface;

struct LogicCtx
{
    int argc;
    char** argv;
    const char* common_component_dir;
    const char* cur_work_dir;
    const char* app_name;
    ConfCenterInterface* conf_center;
    TimerAxisInterface* timer_axis;
    TimeServiceInterface* time_service;
    RandomEngineInterface* random_engine;
    base::MsgDispatcherInterface* msg_dispatcher;
    SchedulerInterface* scheduler;
    LocalLogicInterface* local_logic;

    LogicCtx()
    {
        argc = 0;
        argv = NULL;
        common_component_dir = NULL;
        cur_work_dir = NULL;
        app_name = NULL;
        conf_center = NULL;
        timer_axis = NULL;
        time_service = NULL;
        random_engine = NULL;
        msg_dispatcher = NULL;
        scheduler = NULL;
        local_logic = NULL;
    }
};

class LogicInterface : public ModuleInterface
{
public:
    LogicInterface() : logic_ctx_()
    {
        can_exit_ = false;
        global_logic_ = NULL;
    }

    virtual ~LogicInterface()
    {
    }

    void SetGlobalLogic(global::LogicInterface* global_logic)
    {
        global_logic_ = global_logic;
    }

    virtual int Initialize(const void* ctx)
    {
        if (NULL == ctx)
        {
            return -1;
        }

        logic_ctx_ = *((LogicCtx*) ctx);
        return 0;
    }

    virtual void OnStop()
    {
    }

    virtual void OnReload()
    {
    }

    LogicCtx* GetLogicCtx() const
    {
        return const_cast<LogicCtx*>(&logic_ctx_);
    }

    bool CanExit() const
    {
        return can_exit_;
    }

protected:
    LogicCtx logic_ctx_;
    bool can_exit_;
    global::LogicInterface* global_logic_;
};

class LocalLogicInterface : public LogicInterface
{
public:
    virtual ~LocalLogicInterface()
    {
    }
};
}

#endif // BURDEN_THREADS_INC_BURDEN_LOGIC_INTERFACE_H_
