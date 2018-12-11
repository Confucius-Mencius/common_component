#ifndef TCP_CONN_CENTER_SRC_BUFFER_EVENT_CONN_H_
#define TCP_CONN_CENTER_SRC_BUFFER_EVENT_CONN_H_

#include <event2/bufferevent.h>
#include "mem_util.h"
#include "tcp_conn_center_interface.h"

namespace tcp
{
class ConnCenter;

class BufferEventConn : public ConnInterface
{
    CREATE_FUNC(BufferEventConn)

public:
    BufferEventConn();
    virtual ~BufferEventConn();

    void Release();
    int Initialize(const void* ctx);
    void Finalize();
    int Activate();
    void Freeze();

    void SetSockFD(int sock_fd)
    {
        sock_fd_ = sock_fd;
    }

    void SetBufEvent(struct bufferevent* buf_event)
    {
        buf_event_ = buf_event;
    }

    void SetClientIP(const char* ip)
    {
        client_ip_ = ip;
    }

    void SetClientPort(unsigned short port)
    {
        client_port_ = port;
    }

    void SetCreatedTime(time_t t)
    {
        created_time_ = t;
    }

    void SetConnGUID(int io_thread_idx, ConnID conn_id)
    {
        conn_guid_.io_thread_type = IO_THREAD_TYPE_TCP;
        conn_guid_.io_thread_idx = io_thread_idx;
        conn_guid_.conn_id = conn_id;
    }

    ConnID GetConnID() const
    {
        return conn_guid_.conn_id;
    }

    ///////////////////////// ConnInterface /////////////////////////
    const ConnGUID& GetConnGUID() const override
    {
        return conn_guid_;
    }

    const char* GetClientIP() const override
    {
        return client_ip_.c_str();
    }

    unsigned short GetClientPort() const override
    {
        return client_port_;
    }

    int GetSockFD() const override
    {
        return sock_fd_;
    }

    int Send(const void* data, size_t len) override;

private:
    const ConnCenterCtx* conn_center_ctx_;
    int sock_fd_;
    struct bufferevent* buf_event_;
    std::string client_ip_;
    unsigned short client_port_;
    time_t created_time_;
    ConnGUID conn_guid_;
};
}

#endif // TCP_CONN_CENTER_SRC_BUFFER_EVENT_CONN_H_
