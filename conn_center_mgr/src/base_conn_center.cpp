#include "base_conn_center.h"
#include "container_util.h"
#include "log_util.h"
#include "mem_util.h"

namespace conn_center_mgr
{
BaseConnCenter::BaseConnCenter() : conn_id_seq_(), conn_id_hash_map_(), inactive_conn_mgr_()
{
    max_online_conn_count_ = 0;
}

BaseConnCenter::~BaseConnCenter()
{
}

void BaseConnCenter::Release()
{
    ConnIdHashMap tmp_conn_id_hash_map = conn_id_hash_map_;

    for (ConnIdHashMap::iterator it = tmp_conn_id_hash_map.begin(); it != tmp_conn_id_hash_map.end(); ++it)
    {
        BaseConn* conn = it->second;
        conn_id_seq_.Free(conn->GetConnId());
        conn->Release();
    }

    conn_id_hash_map_.clear();
}

void BaseConnCenter::Finalize()
{
    inactive_conn_mgr_.Finalize();
    FINALIZE_KV_CONTAINER(conn_id_hash_map_);
}

int BaseConnCenter::Activate()
{
    if (inactive_conn_mgr_.Activate() != 0)
    {
        return -1;
    }

    return 0;
}

void BaseConnCenter::Freeze()
{
    inactive_conn_mgr_.Freeze();
    FREEZE_KV_CONTAINER(conn_id_hash_map_);
}

void BaseConnCenter::RemoveConn(BaseConn* conn)
{
    const ConnId conn_id = conn->GetConnId();
    SAFE_DESTROY(conn);

    conn_id_seq_.Free(conn_id);
    conn_id_hash_map_.erase(conn_id);
}
}
