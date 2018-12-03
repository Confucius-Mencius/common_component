#include "inactive_conn_mgr.h"
#include "base_conn_center.h"
#include "log_util.h"

namespace conn_center_mgr
{
InactiveConnMgr::InactiveConnMgr()
{
    conn_center_ = NULL;
}

InactiveConnMgr::~InactiveConnMgr()
{
}

void InactiveConnMgr::OnTimeout(const ConnId& k, BaseConn* const& v, int timeout_sec)
{
    LOG_TRACE("InactiveConnMgr::OnTimeout, k: " << k << ", v: " << v << ", timeout: " << timeout_sec);
    conn_center_->TimeoutLogic(v);
}
}
