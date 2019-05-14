#ifndef GLOBAL_THREAD_INC_GLOBAL_SCHEDULER_INTERFACE_H_
#define GLOBAL_THREAD_INC_GLOBAL_SCHEDULER_INTERFACE_H_

#include <stddef.h>

struct ConnGUID;

namespace proto
{
struct MsgHead;
}

namespace global
{
class SchedulerInterface
{
public:
    virtual ~SchedulerInterface()
    {
    }

    /**
     *
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param work_thread_idx 为-1表示广播
     * @return
     */
    virtual int SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                 const void* msg_body, size_t msg_body_len, int work_thread_idx) = 0;

    /**
     *
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param burden_thread_idx 为-1表示广播
     * @return
     */
    virtual int SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                   const void* msg_body, size_t msg_body_len, int burden_thread_idx) = 0;

    virtual int SendToTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                const void* msg_body, size_t msg_body_len, int tcp_thread_idx) = 0;

    /**
     *
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param tcp_thread_idx 为-1表示广播
     * @return
     */
    virtual int SendToProtoTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                     const void* msg_body, size_t msg_body_len, int proto_tcp_thread_idx) = 0;

    virtual int SendToWebThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                const void* msg_body, size_t msg_body_len, int web_thread_idx) = 0;
};
}

#endif // GLOBAL_THREAD_INC_GLOBAL_SCHEDULER_INTERFACE_H_
