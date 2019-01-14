#ifndef TCP_THREADS_INC_TCP_LOGIC_INTERFACE_H_
#define TCP_THREADS_INC_TCP_LOGIC_INTERFACE_H_

#include "conn_define.h"
#include "module_interface.h"

class ConfCenterInterface;
class TimerAxisInterface;
class ThreadInterface;
struct event_base;

namespace global
{
class LogicInterface;
}

namespace tcp
{
class SchedulerInterface;
class LocalLogicInterface;

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
    LocalLogicInterface* local_logic;
    struct event_base* thread_ev_base;

    LogicCtx()
    {
        argc = 0;
        argv = NULL;
        common_component_dir = NULL;
        cur_working_dir = NULL;
        app_name = NULL;
        conf_center = NULL;
        timer_axis = NULL;
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

    /**
     * 连接管理接口，当有新的客户端连上来时会回调到这里
     * @param conn_guid
     */
    virtual void OnClientConnected(const ConnGUID* conn_guid)
    {
        (void) conn_guid;
    }

    /**
     * 连接管理接口，当有客户端连接断开时会回调到这里，包括服务器主动关闭的连接
     * @param conn_guid
     */
    virtual void OnClientClosed(const ConnGUID* conn_guid)
    {
        (void) conn_guid;
    }

#if defined(USE_BUFFEREVENT)
    virtual void OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len)
    {
        (void) conn_guid;
        (void) data;
        (void) len;
    }
#else
    /**
     *
     * @param closed 如果read返回0，表示对端关闭了，需要将closed置为true。如果read返回-1，且errno为ECONNRESET，也要将closed置为true
     * @param conn_guid
     * @param sock_fd
     * @attention 需要循环读到出错为止
     */
    virtual void OnRecvClientData(bool& closed, const ConnGUID* conn_guid, int sock_fd)
    {
        (void) closed;
        (void) conn_guid;
        (void) sock_fd;
    }
#endif

    virtual void OnTask(const ConnGUID* conn_guid, ThreadInterface* source_thread, const void* data, size_t len)
    {
        (void) conn_guid;
        (void) source_thread;
        (void) data;
        (void) len;
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

#endif // TCP_THREADS_INC_TCP_LOGIC_INTERFACE_H_
