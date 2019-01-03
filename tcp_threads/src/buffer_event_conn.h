#ifndef TCP_THREADS_SRC_BUFFER_EVENT_CONN_H_
#define TCP_THREADS_SRC_BUFFER_EVENT_CONN_H_

#include <event2/bufferevent.h>
#include "conn_define.h"
#include "mem_util.h"

namespace tcp
{
class ConnMgr;

class BufferEventConn
{
    CREATE_FUNC(BufferEventConn)

public:
    BufferEventConn();
    ~BufferEventConn();

    void Release();
    int Initialize(const void* ctx);
    void Finalize();
    int Activate();
    void Freeze();

    void SetCreatedTime(time_t t)
    {
        created_time_ = t;
    }

    void SetSockFD(int sock_fd)
    {
        sock_fd_ = sock_fd;
    }

    int GetSockFD() const
    {
        return sock_fd_;
    }

    void SetBufferEvent(struct bufferevent* buffer_event)
    {
        buffer_event_ = buffer_event;
    }

    void SetClientIP(const char* ip)
    {
        client_ip_ = ip;
    }

    const char* GetClientIP() const
    {
        return client_ip_.c_str();
    }

    void SetClientPort(unsigned short port)
    {
        client_port_ = port;
    }

    unsigned short GetClientPort() const
    {
        return client_port_;
    }

    void SetConnGUID(int io_thread_idx, ConnID conn_id)
    {
        conn_guid_.io_thread_type = IO_THREAD_TYPE_TCP;
        conn_guid_.io_thread_idx = io_thread_idx;
        conn_guid_.conn_id = conn_id;
    }

    const ConnGUID& GetConnGUID() const
    {
        return conn_guid_;
    }

    int Send(const void* data, size_t len);

private:
    time_t created_time_;
    int sock_fd_;
    struct bufferevent* buffer_event_;
    std::string client_ip_;
    unsigned short client_port_;
    ConnGUID conn_guid_;
};
}

#endif // TCP_THREADS_SRC_BUFFER_EVENT_CONN_H_
