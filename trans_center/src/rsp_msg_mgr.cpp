#include "rsp_msg_mgr.h"
#include "trans_center.h"

namespace trans_center
{
RspMsgMgr::RspMsgMgr()
{
    trans_center_ = nullptr;
}

RspMsgMgr::~RspMsgMgr()
{
}

void RspMsgMgr::OnTimeout(const TransID& k, Trans* const& v, int timeout_sec)
{
    LOG_TRACE("NeedRspMsgMgr::OnTimeout, k: " << k << ", v: " << v << ", timeout: " << timeout_sec);
    trans_center_->OnTimeout(k);
}
}
