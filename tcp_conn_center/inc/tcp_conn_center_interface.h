#ifndef TCP_CONN_CENTER_INC_TCP_CONN_CENTER_INTERFACE_H_
#define TCP_CONN_CENTER_INC_TCP_CONN_CENTER_INTERFACE_H_

#include "conn_define.h"
#include "module_interface.h"

class TimerAxisInterface;
struct bufferevent;
struct event;

namespace tcp
{
struct ConnCenterCtx
{
    TimerAxisInterface* timer_axis;
    struct timeval inactive_conn_check_interval;
    int inactive_conn_life;

    ConnCenterCtx()
    {
        timer_axis = NULL;
        inactive_conn_check_interval.tv_sec = inactive_conn_check_interval.tv_usec = 0;
        inactive_conn_life = 0;
    }
};

class ConnInterface
{
public:
    virtual ~ConnInterface()
    {
    }

    virtual const ConnGUID& GetConnGUID() const = 0;
    virtual const char* GetClientIP() const = 0;
    virtual unsigned short GetClientPort() const = 0;
    virtual int GetSockFD() const = 0;
    virtual int Send(const void* data, size_t len) = 0;
};

/**
 * @brief 连接持续一段时间不活跃则回收空闲连接
 */
class ConnInactiveSinkInterface
{
public:
    virtual ~ConnInactiveSinkInterface()
    {
    }

    virtual void OnConnInactive(ConnInterface* conn) = 0;
};

class ConnCenterInterface : public ModuleInterface
{
public:
    virtual ~ConnCenterInterface()
    {
    }

#if defined(USE_BUFFEREVENT)
    // buffer event方式
    virtual ConnInterface* CreateBufferEventConn(int io_thread_idx, int sock_fd, struct bufferevent* buf_event,
            const char* ip, unsigned short port) = 0;
#else
    // 非buffer event方式
    virtual ConnInterface* CreateNormalConn(int io_thread_idx, int sock_fd, struct event* read_event,
                                            const char* ip, unsigned short port) = 0;
#endif

    /**
     * @brief
     * @param sock_fd
     * @note 其中会释放conn对象
     */
    virtual void DestroyConn(int sock_fd) = 0;

    virtual ConnInterface* GetConn(int sock_fd) const = 0;
    virtual ConnInterface* GetConnByID(ConnID conn_id) const = 0;

    /**
     * @brief
     * @param conn_id
     */
    virtual void UpdateConnStatus(ConnID conn_id) = 0;

    virtual int AddConnInactiveSink(ConnInactiveSinkInterface* sink) = 0;
    virtual void RemoveConnInactiveSink(ConnInactiveSinkInterface* sink) = 0;
};
}

#endif // TCP_CONN_CENTER_INC_TCP_CONN_CENTER_INTERFACE_H_
