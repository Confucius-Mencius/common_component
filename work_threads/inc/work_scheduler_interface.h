#ifndef WORK_THREADS_INC_WORK_SCHEDULER_INTERFACE_H_
#define WORK_THREADS_INC_WORK_SCHEDULER_INTERFACE_H_

#include <stddef.h>

struct ConnGUID;

namespace proto
{
struct MsgHead;
}

namespace work
{
class SchedulerInterface
{
public:
    virtual ~SchedulerInterface()
    {
    }

    virtual int SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                                   size_t msg_body_len) = 0;
    
    /**
     * @brief SendToWorkThread
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param work_thread_idx 为-1则由框架自行调度一个work线程
     * @return
     */
    virtual int SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                                 size_t msg_body_len, int work_thread_idx) = 0;

    /**
     * @brief SendToBurdenThread
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param burden_thread_idx 为-1则由框架自行调度一个burden线程
     * @return
     */
    virtual int SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                                   size_t msg_body_len, int burden_thread_idx) = 0;

    /**
     * @brief SendToProtoTCPThread
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param tcp_thread_idx 为-1则由框架自行调度一个proto tcp线程
     * @return
     */
    virtual int SendToProtoTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                                     size_t msg_body_len, int tcp_thread_idx) = 0;
};
}

#endif // WORK_THREADS_INC_WORK_SCHEDULER_INTERFACE_H_
