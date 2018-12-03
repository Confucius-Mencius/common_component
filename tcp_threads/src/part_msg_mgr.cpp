#include "part_msg_mgr.h"
#include "log_util.h"
#include "tcp_thread_sink.h"

namespace tcp
{
PartMsgMgr::PartMsgMgr()
{
    io_thread_sink_ = NULL;
}

PartMsgMgr::~PartMsgMgr()
{
}

void PartMsgMgr::OnTimeout(ConnInterface* const& k, const int& v, int timeout_sec)
{
    LOG_TRACE("PartMsgMgr::OnTimeout, k: " << k << ", v: " << v << ", timeout: " << timeout_sec);
    io_thread_sink_->CloseConn(v);
}
}
