#ifndef CONN_CENTER_MGR_SRC_BASE_CONN_CENTER_H_
#define CONN_CENTER_MGR_SRC_BASE_CONN_CENTER_H_

#include "hash_container.h"
#include "inactive_conn_mgr.h"

namespace conn_center_mgr
{
class BaseConnCenter
{
public:
    BaseConnCenter();
    virtual ~BaseConnCenter();

public:
    virtual void Release();
    virtual int Initialize(const void* ctx) = 0;
    virtual void Finalize();
    virtual int Activate();
    virtual void Freeze();
    virtual void RemoveLogic(BaseConn* conn) = 0;
    virtual void TimeoutLogic(BaseConn* conn) = 0;

protected:
    void RemoveConn(BaseConn* conn);

protected:
    typedef __hash_map<ConnId, BaseConn*> ConnIdHashMap;

    ConnIdSeq conn_id_seq_;
    ConnIdHashMap conn_id_hash_map_;

    InactiveConnMgr inactive_conn_mgr_;
    int max_online_conn_count_; // 每个负责IO的线程中同时在线的conn最大数，统计用
};
}

#endif // CONN_CENTER_MGR_SRC_BASE_CONN_CENTER_H_
