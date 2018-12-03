#ifndef CONN_CENTER_MGR_SRC_CONN_CENTER_MGR_H_
#define CONN_CENTER_MGR_SRC_CONN_CENTER_MGR_H_

#include <set>
#include "conn_center_mgr_interface.h"
#include "tcp_conn_center.h"
#include "http_conn_center.h"
#include "udp_conn_center.h"

namespace conn_center_mgr
{
class ConnCenterMgr : public ConnCenterMgrInterface
{
public:
    ConnCenterMgr();
    virtual ~ConnCenterMgr();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ConnCenterMgrInterface /////////////////////////
    tcp::ConnCenterInterface* CreateTcpConnCenter(const tcp::ConnCenterCtx* ctx) override;
    http::ConnCenterInterface* CreateHttpConnCenter(const http::ConnCenterCtx* ctx) override;
    udp::ConnCenterInterface* CreateUdpConnCenter(const udp::ConnCenterCtx* ctx) override;

private:
    typedef std::set<tcp::ConnCenter*> TcpConnCenterSet;
    typedef std::set<http::ConnCenter*> HttpConnCenterSet;
    typedef std::set<udp::ConnCenter*> UdpConnCenterSet;

    TcpConnCenterSet tcp_conn_center_set_;
    HttpConnCenterSet http_conn_center_set_;
    UdpConnCenterSet udp_conn_center_set_;
};
}

#endif // CONN_CENTER_MGR_SRC_CONN_CENTER_MGR_H_
