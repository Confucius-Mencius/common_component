#ifndef CLIENT_CENTER_MGR_INC_CLIENT_CENTER_MGR_INTERFACE_H_
#define CLIENT_CENTER_MGR_INC_CLIENT_CENTER_MGR_INTERFACE_H_

#include "module_interface.h"
#include "http_client_center_interface.h"
#include "proto_tcp_client_center_interface.h"

class ClientCenterMgrInterface : public ModuleInterface
{
public:
    virtual ~ClientCenterMgrInterface()
    {
    }

    virtual tcp::proto::ClientCenterInterface* CreateProtoTCPClientCenter(const tcp::proto::ClientCenterCtx* ctx) = 0;
    virtual http::ClientCenterInterface* CreateHTTPClientCenter(const http::ClientCenterCtx* ctx) = 0;
};

#endif // CLIENT_CENTER_MGR_INC_CLIENT_CENTER_MGR_INTERFACE_H_
