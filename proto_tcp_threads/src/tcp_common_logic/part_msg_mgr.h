#ifndef PROTO_TCP_THREADS_SRC_TCP_COMMON_LOGIC_PART_MSG_MGR_H_
#define PROTO_TCP_THREADS_SRC_TCP_COMMON_LOGIC_PART_MSG_MGR_H_

#include "conn.h"
#include "record_timeout_mgr.h"
#include "scheduler.h"

namespace tcp
{
namespace proto
{
// ConnInterface* conn -> int sock_fd
class PartMsgMgr : public RecordTimeoutMgr<ConnInterface*, std::hash<ConnInterface*>, ConnGUID>
{
public:
    PartMsgMgr();
    virtual ~PartMsgMgr();

    void SetScheduler(Scheduler* scheduler)
    {
        scheduler_ = scheduler;
    }

protected:
    ///////////////////////// RecordTimeoutMgr /////////////////////////
    void OnTimeout(ConnInterface* const& k, const ConnGUID& v, int timeout_sec) override;

private:
    Scheduler* scheduler_;
};
}
}

#endif // PROTO_TCP_THREADS_SRC_TCP_COMMON_LOGIC_PART_MSG_MGR_H_
