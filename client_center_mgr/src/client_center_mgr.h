#ifndef CLIENT_CENTER_MGR_SRC_CLIENT_CENTER_MGR_H_
#define CLIENT_CENTER_MGR_SRC_CLIENT_CENTER_MGR_H_

#include "client_center_mgr_interface.h"
#include "module_loader.h"
#include "http_client_center.h"
#include "tcp_client_center.h"
#include "udp_client_center.h"

namespace client_center_mgr
{
class ClientCenterMgr : public ClientCenterMgrInterface
{
public:
    ClientCenterMgr();
    virtual ~ClientCenterMgr();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ClientCenterMgrInterface /////////////////////////
    tcp::ClientCenterInterface* CreateTcpClientCenter(const tcp::ClientCenterCtx* ctx) override;
    http::ClientCenterInterface* CreateHttpClientCenter(const http::ClientCenterCtx* ctx) override;
    udp::ClientCenterInterface* CreateUdpClientCenter(const udp::ClientCenterCtx* ctx) override;

private:
//    ClientCenterMgrCtx client_center_mgr_ctx_;

    typedef std::set<tcp::ClientCenter*> TcpClientCenterSet;
    typedef std::set<http::ClientCenter*> HttpClientCenterSet;
    typedef std::set<udp::ClientCenter*> UdpClientCenterSet;

    TcpClientCenterSet tcp_client_center_set_;
    HttpClientCenterSet http_client_center_set_;
    UdpClientCenterSet udp_client_center_set_;
};
}

#endif // CLIENT_CENTER_MGR_SRC_CLIENT_CENTER_MGR_H_
