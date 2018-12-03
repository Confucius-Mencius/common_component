#ifndef GLOBAL_THREAD_INC_GLOBAL_SCHEDULER_INTERFACE_H_
#define GLOBAL_THREAD_INC_GLOBAL_SCHEDULER_INTERFACE_H_

#include "scheduler_interface.h"

namespace global
{
class SchedulerInterface : public base::SchedulerInterface
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
     * @param work_thread_idx -1表示广播，为广播时返回值无效，不用判断
     * @return
     */
    virtual int SendToWorkThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                 size_t msg_body_len, int work_thread_idx) = 0;

    /**
     *
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param burden_thread_idx -1表示广播，为广播时返回值无效，不用判断
     * @return
     */
    virtual int SendToBurdenThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                   size_t msg_body_len, int burden_thread_idx) = 0;

    /**
     *
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param tcp_thread_idx -1表示广播，为广播时返回值无效，不用判断
     * @return
     */
    virtual int SendToTcpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len, int tcp_thread_idx) = 0;

    /**
     *
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param http_thread_idx 不能为-1，回复给指定http线程
     * @return
     */
    virtual int SendToHttpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                 size_t msg_body_len, int http_thread_idx) = 0;

    /**
     *
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param udp_thread_idx -1表示广播，为广播时返回值无效，不用判断
     * @return
     */
    virtual int SendToUdpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len, int udp_thread_idx) = 0;
};
}

#endif // GLOBAL_THREAD_INC_GLOBAL_SCHEDULER_INTERFACE_H_
