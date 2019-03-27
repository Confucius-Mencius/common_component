#ifndef GLOBAL_THREAD_INC_GLOBAL_LOGIC_INTERFACE_H_
#define GLOBAL_THREAD_INC_GLOBAL_LOGIC_INTERFACE_H_

#include <stddef.h>
#include "module_interface.h"

class ConfCenterInterface;
class TimerAxisInterface;

namespace proto
{
class MsgDispatcherInterface;
}

struct ConnGUID;

namespace global
{
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
    SchedulerInterface* scheduler;
    ::proto::MsgDispatcherInterface* msg_dispatcher;
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
        scheduler = NULL;
        msg_dispatcher = NULL;
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

        logic_ctx_ = *(static_cast<const LogicCtx*>(ctx));
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
