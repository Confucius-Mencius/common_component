#ifndef CLIENT_CENTER_MGR_SRC_CLIENT_CENTER_MGR_H_
#define CLIENT_CENTER_MGR_SRC_CLIENT_CENTER_MGR_H_

#include "client_center_mgr_interface.h"
#include "module_loader.h"
#include "http_client_center.h"
#include "proto_tcp_client_center.h"

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
    tcp::proto::ClientCenterInterface* CreateProtoTCPClientCenter(const tcp::proto::ClientCenterCtx* ctx) override;
    http::ClientCenterInterface* CreateHTTPClientCenter(const http::ClientCenterCtx* ctx) override;

private:
    typedef std::set<tcp::proto::ClientCenter*> ProtoTCPClientCenterSet;
    typedef std::set<http::ClientCenter*> HTTPClientCenterSet;

    ProtoTCPClientCenterSet proto_tcp_client_center_set_;
    HTTPClientCenterSet http_client_center_set_;
};
}

#endif // CLIENT_CENTER_MGR_SRC_CLIENT_CENTER_MGR_H_
