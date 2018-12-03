#ifndef CONN_CENTER_MGR_SRC_UDP_CONN_H_
#define CONN_CENTER_MGR_SRC_UDP_CONN_H_

// 代理外部发来的udp连接

#include <string.h>
#include "base_conn.h"
#include "mem_util.h"
#include "udp_conn_center_interface.h"

namespace udp
{
class Conn : public conn_center_mgr::BaseConn, public ConnInterface
{
    CREATE_FUNC(Conn);

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

    int Send(const MsgHead& msg_head, const void* msg_body, size_t msg_body_len, int total_retry) override;

public:
    void SetSockFd(int sock_fd)
    {
        sock_fd_ = sock_fd;
    }

    int GetSockFd() const
    {
        return sock_fd_;
    }

    void SetClientIp(const char* client_ip)
    {
        client_ip_ = client_ip;
    }

    void SetClientPort(unsigned short client_port)
    {
        client_port_ = client_port;
    }

    void SetClientAddr(const struct sockaddr_in* client_addr)
    {
        memcpy(&client_addr_, client_addr, sizeof(client_addr_));
    }

private:
    const ConnCenterCtx* conn_center_ctx_;
    int sock_fd_;
    char* send_buf_;
    struct sockaddr_in client_addr_;
};
}

#endif // CONN_CENTER_MGR_SRC_UDP_CONN_H_
