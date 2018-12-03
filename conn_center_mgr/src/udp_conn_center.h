#ifndef CONN_CENTER_MGR_SRC_UDP_CONN_CENTER_H_
#define CONN_CENTER_MGR_SRC_UDP_CONN_CENTER_H_

#include <set>
#include "base_conn_center.h"
#include "mem_util.h"
#include "udp_conn.h"

namespace udp
{
class ConnCenter : public conn_center_mgr::BaseConnCenter, public ConnCenterInterface
{
    CREATE_FUNC(ConnCenter);

public:
    ConnCenter();
    virtual ~ConnCenter();

    ///////////////////////// BaseConnCenter /////////////////////////
    void Release() override;
    int Initialize(const void* ctx) override;
    void RemoveLogic(conn_center_mgr::BaseConn* conn) override;
    void TimeoutLogic(conn_center_mgr::BaseConn* conn) override;

    ///////////////////////// ConnCenterInterface /////////////////////////
    ConnInterface* CreateConn(int io_thread_idx, int sock_fd, const struct sockaddr_in* client_addr,
                              const char* ip, unsigned short port) override;
    void RemoveConn(const char* ip, unsigned short port) override;
    ConnInterface* GetConn(const char* ip, unsigned short port) override;
    ConnInterface* GetConn(const char* ip_port) override;
    ConnInterface* GetConnByConnId(ConnId conn_id) const override;
    void UpdateConnStatus(ConnId conn_id) override;

    int AddConnTimeoutSink(ConnTimeoutSinkInterface* conn_timeout_sink) override;
    void RemoveConnTimeoutSink(ConnTimeoutSinkInterface* conn_timeout_sink) override;

private:
    ConnCenterCtx conn_center_ctx_;

    typedef __hash_map<std::string, Conn*> ConnHashMap; // client ip and port ->
    ConnHashMap conn_hash_map_;

    typedef std::set<ConnTimeoutSinkInterface*> ConnTimeoutSinkSet;
    ConnTimeoutSinkSet conn_timeout_sink_set_;
};
}

#endif // CONN_CENTER_MGR_SRC_UDP_CONN_CENTER_H_
