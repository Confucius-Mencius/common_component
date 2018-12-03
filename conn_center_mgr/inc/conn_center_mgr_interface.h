#ifndef CONN_CENTER_MGR_INC_CONN_CENTER_MGR_INTERFACE_H_
#define CONN_CENTER_MGR_INC_CONN_CENTER_MGR_INTERFACE_H_

#include "module_interface.h"
#include "http_conn_center_interface.h"
#include "tcp_conn_center_interface.h"
#include "udp_conn_center_interface.h"

class ConnCenterMgrInterface : public ModuleInterface
{
public:
    virtual ~ConnCenterMgrInterface()
    {
    }

    virtual tcp::ConnCenterInterface* CreateTcpConnCenter(const tcp::ConnCenterCtx* conn_center_ctx) = 0;
    virtual http::ConnCenterInterface* CreateHttpConnCenter(const http::ConnCenterCtx* conn_center_ctx) = 0;
    virtual udp::ConnCenterInterface* CreateUdpConnCenter(const udp::ConnCenterCtx* conn_center_ctx) = 0;

    // TODO websocket
//    virtual websocket::ConnCenterInterface* CreateWebSocketConnCenter(const websocket::ConnCenterCtx* conn_center_ctx) = 0;
};

#endif // CONN_CENTER_MGR_INC_CONN_CENTER_MGR_INTERFACE_H_
