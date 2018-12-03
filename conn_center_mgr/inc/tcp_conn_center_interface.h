#ifndef CONN_CENTER_MGR_INC_TCP_CONN_CENTER_INTERFACE_H_
#define CONN_CENTER_MGR_INC_TCP_CONN_CENTER_INTERFACE_H_

#include "common_define.h"
#include "msg_define.h"

class TimerAxisInterface;
class MsgCodecInterface;
struct bufferevent;
struct event;

namespace tcp
{
struct ConnCenterCtx
{
    TimerAxisInterface* timer_axis;
    MsgCodecInterface* msg_codec;
    struct timeval inactive_conn_check_interval;
    int inactive_conn_life;
    size_t max_msg_body_len;

    ConnCenterCtx()
    {
        timer_axis = NULL;
        msg_codec = NULL;
        inactive_conn_check_interval.tv_sec = inactive_conn_check_interval.tv_usec = 0;
        inactive_conn_life = 0;
        max_msg_body_len = 0;
    }
};

class ConnInterface
{
public:
    virtual ~ConnInterface()
    {
    }

    virtual const ConnGuid& GetConnGuid() const = 0;
    virtual const char* GetClientIp() const = 0;
    virtual unsigned short GetClientPort() const = 0;
    virtual int GetSockFd() const = 0;
    virtual int Send(const MsgHead& msg_head, const void* msg_body, size_t msg_body_len, int total_retry) = 0;
    virtual int Send(const void* msg, size_t msg_len, int total_retry) = 0;
};

class ConnTimeoutSinkInterface
{
public:
	virtual ~ConnTimeoutSinkInterface()
	{
	}

	virtual void OnConnTimeout(ConnInterface* conn) = 0;
};

class ConnCenterInterface
{
public:
    virtual ~ConnCenterInterface()
    {
    }

    // buffer event方式
    virtual ConnInterface* CreateBufferEventConn(int io_thread_idx, int sock_fd, struct bufferevent* buf_event,
                                                 const char* ip, unsigned short port) = 0;

    // 非buffer event方式
    virtual ConnInterface* CreateNormalConn(int io_thread_idx, int sock_fd, struct event* read_event,
                                            const char* ip, unsigned short port) = 0;
    /**
     * @brief
     * @param sock_fd
     * @note 其中会释放conn对象
     */
    virtual void RemoveConn(int sock_fd) = 0;

    virtual ConnInterface* GetConn(int sock_fd) const = 0;
    virtual ConnInterface* GetConnByConnId(ConnId conn_id) const = 0;

    /**
     * @brief 通知该连接是活动的
     * @param conn_id
     */
    virtual void UpdateConnStatus(ConnId conn_id) = 0;

    virtual int AddConnTimeoutSink(ConnTimeoutSinkInterface* conn_timeout_sink) = 0;
    virtual void RemoveConnTimeoutSink(ConnTimeoutSinkInterface* conn_timeout_sink) = 0;
};
}

#endif // CONN_CENTER_MGR_INC_TCP_CONN_CENTER_INTERFACE_H_
