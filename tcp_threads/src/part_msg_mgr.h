#ifndef TCP_THREADS_SRC_PART_MSG_MGR_H_
#define TCP_THREADS_SRC_PART_MSG_MGR_H_

#include "record_timeout_mgr.h"
#include "tcp_conn_center_interface.h"

namespace tcp
{
class ThreadSink;

// ConnInterface* conn -> int sock_fd
class PartMsgMgr : public RecordTimeoutMgr<ConnInterface*, std::hash<ConnInterface*>, int>
{
public:
    PartMsgMgr();
    virtual ~PartMsgMgr();

    void SetIOThreadSink(ThreadSink* sink)
    {
        io_thread_sink_ = sink;
    }

protected:
    ///////////////////////// RecordAccessMgrTemplate /////////////////////////
    void OnTimeout(ConnInterface* const& k, const int& v, int timeout_sec) override;

private:
    ThreadSink* io_thread_sink_;
};
}

#endif // TCP_THREADS_SRC_PART_MSG_MGR_H_
