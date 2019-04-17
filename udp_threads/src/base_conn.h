#ifndef UDP_THREADS_SRC_BASE_CONN_H_
#define UDP_THREADS_SRC_BASE_CONN_H_

#include <event2/util.h>
#include "conn.h"

namespace udp
{
class ConnCenter;
class ThreadSink;

class BaseConn : public ConnInterface
{
public:
    BaseConn();
    virtual ~BaseConn();

    virtual void Release() = 0;
    virtual int Initialize(const void* ctx) = 0;
    virtual void Finalize() = 0;
    virtual int Activate() = 0;
    virtual void Freeze() = 0;

    virtual int Send(const void* data, size_t len) = 0;

    ///////////////////////// ConnInterface /////////////////////////
    const ConnGUID* GetConnGUID() const
    {
        return &conn_guid_;
    }

    const char* GetClientIP() const override
    {
        return client_ip_.c_str();
    }

    unsigned short GetClientPort() const override
    {
        return client_port_;
    }

    std::string& AppendData(const char* data, size_t len) override
    {
        return data_.append(data, len);
    }

    void ClearData() override
    {
        data_.clear();
    }

    std::string& GetData() override
    {
        return data_;
    }

    void SetConnCenter(ConnCenter* conn_center)
    {
        conn_center_ = conn_center;
    }

    ConnCenter* GetConnCenter()
    {
        return conn_center_;
    }

    void SetCreatedTime(time_t t)
    {
        created_time_ = t;
    }

    void SetConnGUID(IOType io_type, int io_thread_idx, ConnID conn_id)
    {
        conn_guid_.io_type = io_type;
        conn_guid_.io_thread_idx = io_thread_idx;
        conn_guid_.conn_id = conn_id;
    }

    void SetClientIP(const char* ip)
    {
        client_ip_ = ip;
    }

    void SetClientPort(unsigned short port)
    {
        client_port_ = port;
    }

    void SetSockFD(int sock_fd)
    {
        sock_fd_ = sock_fd;
    }

    int GetSockFD() const
    {
        return sock_fd_;
    }

    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

protected:
    ConnCenter* conn_center_;
    time_t created_time_;
    ConnGUID conn_guid_;
    std::string client_ip_;
    unsigned short client_port_;
    evutil_socket_t sock_fd_;
    ThreadSink* thread_sink_;
    std::string data_;
};
}

#endif // UDP_THREADS_SRC_BASE_CONN_H_
