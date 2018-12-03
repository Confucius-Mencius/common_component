#ifndef CONN_CENTER_MGR_SRC_HTTP_CONN_CENTER_H_
#define CONN_CENTER_MGR_SRC_HTTP_CONN_CENTER_H_

#include "base_conn_center.h"
#include "http_conn.h"
#include "mem_util.h"

namespace http
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
    void Finalize() override;
    int Activate() override;
    void Freeze() override;
    void RemoveLogic(conn_center_mgr::BaseConn* conn) override;
    void TimeoutLogic(conn_center_mgr::BaseConn* conn) override;

    ///////////////////////// ConnCenterInterface /////////////////////////
    ConnInterface* CreateConn(int io_thread_idx, struct evhttp_connection* evhttp_conn, bool https) override;
    void RemoveConn(struct evhttp_connection* evhttp_conn) override;
    ConnInterface* GetConn(struct evhttp_connection* evhttp_conn) const override;
    ConnInterface* GetConnByConnId(ConnId conn_id) const override;

private:
    ConnCenterCtx conn_center_ctx_;

    typedef __hash_map<struct evhttp_connection*, Conn*> ConnHashMap;
    ConnHashMap conn_hash_map_;
};
}

#endif // CONN_CENTER_MGR_SRC_HTTP_CONN_CENTER_H_
