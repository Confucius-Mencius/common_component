#ifndef CLIENT_CENTER_MGR_SRC_UDP_CLIENT_H_
#define CLIENT_CENTER_MGR_SRC_UDP_CLIENT_H_

#include <event2/event.h>
#include "mem_util.h"
#include "module_interface.h"
#include "trans_center_interface.h"
#include "udp_client_center_interface.h"

namespace udp
{
class Client : public ModuleInterface, public ClientInterface
{
    CREATE_FUNC(Client);

    static void ReadCallback(evutil_socket_t fd, short events, void* arg);

public:
    Client();
    virtual ~Client();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ClientInterface /////////////////////////
    const Peer& GetPeer() const override
    {
        return peer_;
    }

    TransId Send(const MsgHead& msg_head, const void* msg_body, size_t msg_body_len,
                 const base::AsyncCtx* async_ctx) override;

public:
    void SetPeer(const Peer& peer);

    char* GetRecvBuf() const
    {
        return recv_buf_;
    }

private:
    int InitSocket();
    void OnRecvPeerMsg(evutil_socket_t fd, const struct sockaddr_in* peer_addr, const char* total_msg_buf,
                       size_t total_msg_len);

private:
    const ClientCenterCtx* client_center_ctx_;

    char* send_buf_;
    char* recv_buf_;

    Peer peer_;
    struct sockaddr_in peer_addr_;

    evutil_socket_t sock_fd_;
    struct event* udp_event_;
};
}

#endif // CLIENT_CENTER_MGR_SRC_UDP_CLIENT_H_
