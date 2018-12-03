#ifndef CONN_CENTER_MGR_SRC_TCP_CONN_H_
#define CONN_CENTER_MGR_SRC_TCP_CONN_H_

// 代理外部发来的tcp连接

#include <list>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include "base_conn.h"
#include "mem_util.h"
#include "msg_codec_interface.h"
#include "tcp_conn_center_interface.h"

namespace tcp
{
class ConnCenter;

class Conn : public conn_center_mgr::BaseConn, public ConnInterface
{
    CREATE_FUNC(Conn);
    static void WriteCallback(evutil_socket_t fd, short events, void* arg);

public:
    Conn();
    virtual ~Conn();

    ///////////////////////// BaseConn /////////////////////////
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ConnInterface /////////////////////////
    const ConnGuid& GetConnGuid() const override
    {
        return conn_guid_;
    }

    const char* GetClientIp() const override
    {
        return client_ip_.c_str();
    }

    unsigned short GetClientPort() const override
    {
        return client_port_;
    }

    int GetSockFd() const override
    {
        return sock_fd_;
    }

    int Send(const MsgHead& msg_head, const void* msg_body, size_t msg_body_len, int total_retry) override;
    int Send(const void* msg, size_t msg_len, int total_retry) override;

public:
    void SetSockFd(int sock_fd)
    {
        sock_fd_ = sock_fd;
    }

    void SetBufEvent(struct bufferevent* buf_event)
    {
        buf_event_ = buf_event;
    }

    void SetClientIp(const char* ip)
    {
        client_ip_ = ip;
    }

    void SetClientPort(unsigned short port)
    {
        client_port_ = port;
    }

    void SetReadEvent(struct event* read_event)
    {
        read_event_ = read_event;
    }

private:
    const ConnCenterCtx* conn_center_ctx_;
    int sock_fd_;
    struct bufferevent* buf_event_;
    char* send_buf_;
    struct event* read_event_;

    typedef std::list<std::string> SendList;
    SendList send_list_;

    struct event* write_event_;
};
}

#endif // CONN_CENTER_MGR_SRC_TCP_CONN_H_
