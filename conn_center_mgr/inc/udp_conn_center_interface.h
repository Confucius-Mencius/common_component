#ifndef CONN_CENTER_MGR_INC_UDP_CONN_CENTER_INTERFACE_H_
#define CONN_CENTER_MGR_INC_UDP_CONN_CENTER_INTERFACE_H_

#include <netinet/in.h>
#include "common_define.h"
#include "msg_define.h"

class TimerAxisInterface;
class MsgCodecInterface;

namespace udp
{
struct ConnCenterCtx
{
    TimerAxisInterface* timer_axis;
    MsgCodecInterface* msg_codec;
    struct timeval inactive_conn_check_interval;
    int inactive_conn_life;
    size_t max_udp_msg_len;

    ConnCenterCtx()
    {
        timer_axis = NULL;
        msg_codec = NULL;
        inactive_conn_check_interval.tv_sec = inactive_conn_check_interval.tv_usec = 0;
        inactive_conn_life = 0;
        max_udp_msg_len = 0;
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
    virtual int Send(const MsgHead& msg_head, const void* msg_body, size_t msg_body_len, int total_retry) = 0;
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

    virtual ConnInterface* CreateConn(int io_thread_idx, int sock_fd, const struct sockaddr_in* client_addr,
                                      const char* ip, unsigned short port) = 0;

    /**
     * @brief
     * @param
     * @param
     * @note 其中会释放conn对象
     */
    virtual void RemoveConn(const char* ip, unsigned short port) = 0;

    virtual ConnInterface* GetConn(const char* ip, unsigned short port) = 0;
    virtual ConnInterface* GetConn(const char* ip_port) = 0;
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

#endif // CONN_CENTER_MGR_INC_UDP_CONN_CENTER_INTERFACE_H_
