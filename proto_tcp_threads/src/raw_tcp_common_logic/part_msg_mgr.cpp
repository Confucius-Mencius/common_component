#include "part_msg_mgr.h"
#include "log_util.h"

namespace tcp
{
namespace proto
{
PartMsgMgr::PartMsgMgr()
{
    scheduler_ = nullptr;
}

PartMsgMgr::~PartMsgMgr()
{
}

void PartMsgMgr::OnTimeout(ConnInterface* const& k, const ConnGUID& v, int timeout_sec)
{
    LOG_TRACE("PartMsgMgr::OnTimeout, k: " << k << ", v: " << v << ", timeout: " << timeout_sec);
    scheduler_->CloseClient(&v);
}
}
}
