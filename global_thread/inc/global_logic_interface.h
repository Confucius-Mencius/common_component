#ifndef GLOBAL_THREAD_INC_GLOBAL_LOGIC_INTERFACE_H_
#define GLOBAL_THREAD_INC_GLOBAL_LOGIC_INTERFACE_H_

#include <atomic>
#include "module_interface.h"

class ConfCenterInterface;
class TimerAxisInterface;
struct event_base;

namespace proto
{
class MsgDispatcherInterface;
}

namespace global
{
class SchedulerInterface;
class LogicInterface;

struct LogicCtx
{
    int argc;
    char** argv;
    const char* common_component_dir;
    const char* cur_working_dir;
    const char* app_name;
    ConfCenterInterface* conf_center;
    TimerAxisInterface* timer_axis;
    SchedulerInterface* scheduler;
    ::proto::MsgDispatcherInterface* msg_dispatcher;
    LogicInterface* global_logic;
    struct event_base* thread_ev_base;

    LogicCtx()
    {
        argc = 0;
        argv = nullptr;
        common_component_dir = nullptr;
        cur_working_dir = nullptr;
        app_name = nullptr;
        conf_center = nullptr;
        timer_axis = nullptr;
        scheduler = nullptr;
        msg_dispatcher = nullptr;
        global_logic = nullptr;
        thread_ev_base = nullptr;
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
        if (nullptr == ctx)
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
    std::atomic_bool can_exit_;
};
}

#endif // GLOBAL_THREAD_INC_GLOBAL_LOGIC_INTERFACE_H_
