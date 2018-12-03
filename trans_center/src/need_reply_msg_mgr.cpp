#include "need_reply_msg_mgr.h"
#include "trans_center.h"

namespace trans_center
{
NeedReplyMsgMgr::NeedReplyMsgMgr()
{
    trans_center_ = NULL;
}

NeedReplyMsgMgr::~NeedReplyMsgMgr()
{
}

void NeedReplyMsgMgr::OnTimeout(const TransId& k, Trans* const& v, int timeout_sec)
{
    LOG_TRACE("NeedReplyMsgMgr::OnTimeout, k: " << k << ", v: " << v << ", timeout: " << timeout_sec);
    trans_center_->TimeoutLogic(k);
}
}
