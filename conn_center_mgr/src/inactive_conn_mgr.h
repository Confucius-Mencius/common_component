#ifndef CONN_CENTER_MGR_SRC_INACTIVE_CONN_MGR_H_
#define CONN_CENTER_MGR_SRC_INACTIVE_CONN_MGR_H_

#include "base_conn.h"
#include "record_timeout_mgr.h"

namespace conn_center_mgr
{
class BaseConnCenter;

class InactiveConnMgr : public RecordTimeoutMgr<ConnId, std::hash<ConnId>, BaseConn*>
{
public:
    InactiveConnMgr();
    virtual ~InactiveConnMgr();

    void SetConnCenter(BaseConnCenter* conn_center)
    {
        conn_center_ = conn_center;
    }

protected:
    ///////////////////////// RecordTimeoutMgr<ConnId, std::hash<ConnId>, BaseConn*> /////////////////////////
    void OnTimeout(const ConnId& k, BaseConn* const& v, int timeout_sec) override;

private:
    BaseConnCenter* conn_center_;
};
}

#endif // CONN_CENTER_MGR_SRC_INACTIVE_CONN_MGR_H_
