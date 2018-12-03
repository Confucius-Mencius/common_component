#ifndef GLOBAL_THREAD_INC_GLOBAL_LOGIC_INTERFACE_H_
#define GLOBAL_THREAD_INC_GLOBAL_LOGIC_INTERFACE_H_

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
class ClientMgrInterface;
class SchedulerInterface;
class LogicInterface;

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
    LogicInterface* global_logic;

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
        global_logic = NULL;
    }
};

class LogicInterface : public ModuleInterface
{
public:
    LogicInterface() : logic_ctx_()
    {
        can_exit_ = false;
    }

    virtual ~LogicInterface()
    {
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
};
}

#endif // GLOBAL_THREAD_INC_GLOBAL_LOGIC_INTERFACE_H_
