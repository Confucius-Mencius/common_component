#ifndef TCP_THREADS_INC_TCP_LOGIC_INTERFACE_H_
#define TCP_THREADS_INC_TCP_LOGIC_INTERFACE_H_

#include <stddef.h>
#include "module_interface.h"

class ConfCenterInterface;
class TimerAxisInterface;
class TimeServiceInterface;
class RandomEngineInterface;
struct event_base;

namespace base
{
class MsgDispatcherInterface;
}

struct ConnGuid;

namespace global
{
class LogicInterface;
}

namespace tcp
{
class ConnCenterInterface;
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
    ConnCenterInterface* conn_center;
    base::MsgDispatcherInterface* msg_dispatcher;
    SchedulerInterface* scheduler;
    LocalLogicInterface* local_logic;
    struct event_base* thread_ev_base;

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
        conn_center = NULL;
        msg_dispatcher = NULL;
        scheduler = NULL;
        local_logic = NULL;
        thread_ev_base = NULL;
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

    /**
     * 连接管理接口，当有新的客户端连上来时会回调到这里
     * @param conn_guid
     */
    virtual void OnClientConnected(const ConnGuid* conn_guid)
    {
    }

    /**
     * 连接管理接口，当有客户端连接断开时会回调到这里，包括服务器主动关闭该连接
     * @param conn_guid
     */
    virtual void OnClientClosed(const ConnGuid* conn_guid)
    {
    }

    /**
     * raw tcp数据接收接口
     */
#if defined(USE_BUFFEREVENT)

    virtual void OnRecvClientRawData(const ConnGuid* conn_guid, const void* data, size_t data_len)
    {
    }

#else
    /**
     *
     * @param closed 如果read返回0，表示对端关闭了，需要将closed置为true。如果read返回-1，且errno为ECONNRESET，也要将closed置为true
     * @param conn_guid
     * @param sock_fd
     * @attention 需要循环读到出错为止
     */
    virtual void OnClientRawData(bool& closed, const ConnGuid* conn_guid, int sock_fd)
    {
    }
#endif

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

#endif // TCP_THREADS_INC_TCP_LOGIC_INTERFACE_H_
