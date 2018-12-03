#ifndef CLIENT_CENTER_MGR_INC_CLIENT_CENTER_MGR_INTERFACE_H_
#define CLIENT_CENTER_MGR_INC_CLIENT_CENTER_MGR_INTERFACE_H_

#include "module_interface.h"
#include "http_client_center_interface.h"
#include "tcp_client_center_interface.h"
#include "udp_client_center_interface.h"

//class TimerAxisInterface;
//class TransCenterInterface;

//struct ClientCenterMgrCtx
//{
//    const char* common_component_dir;
//    TimerAxisInterface* timer_axis;
//    TransCenterInterface* trans_center;
//
//    ClientCenterMgrCtx()
//    {
//        common_component_dir = NULL;
//        timer_axis = NULL;
//        trans_center = NULL;
//    }
//};

class ClientCenterMgrInterface : public ModuleInterface
{
public:
    virtual ~ClientCenterMgrInterface()
    {
    }

    virtual tcp::ClientCenterInterface* CreateTcpClientCenter(const tcp::ClientCenterCtx* ctx) = 0;
    virtual http::ClientCenterInterface* CreateHttpClientCenter(const http::ClientCenterCtx* ctx) = 0;
    virtual udp::ClientCenterInterface* CreateUdpClientCenter(const udp::ClientCenterCtx* ctx) = 0;

    // TODO websocket
//    virtual websocket::ClientCenterInterface* CreateWebSocketClientCenter(const websocket::ClientCenterCtx* ctx) = 0;
};

#endif // CLIENT_CENTER_MGR_INC_CLIENT_CENTER_MGR_INTERFACE_H_
