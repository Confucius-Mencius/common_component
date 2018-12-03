#ifndef WORK_THREADS_INC_WORK_SCHEDULER_INTERFACE_H_
#define WORK_THREADS_INC_WORK_SCHEDULER_INTERFACE_H_

#include "scheduler_interface.h"

namespace work
{
class SchedulerInterface : public base::SchedulerInterface
{
public:
    virtual ~SchedulerInterface()
    {
    }

    virtual int SendToWorkThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                 size_t msg_body_len, int work_thread_idx) = 0;

    /**
     *
     * @param task normal task type
     * @param params
     * @param burden_thread_idx 为-1则由框架轮询选择
     * @return
     */
    virtual int SendToBurdenThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                   size_t msg_body_len, int burden_thread_idx) = 0;

    virtual int SendToGlobalThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                   size_t msg_body_len) = 0;

    /**
     * @brief send msg to client
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @return
     */
    virtual int SendToTcpClient(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len) = 0;

    virtual int SendRawToTcpClient(const ConnGuid* conn_guid, const void* msg, size_t msg_len) = 0;

    /**
     * @brief 断开与客户端的连接
     * @param conn_guid
     * @return
     */
    virtual int CloseTcpClient(const ConnGuid* conn_guid) = 0;

    virtual int SendToUdpClient(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len) = 0;

    virtual int CloseUdpClient(const ConnGuid* conn_guid) = 0;

    // 上一级线程组，自己这组线程，下一级线程组，idx都可为-1，由框架均匀调度
    virtual int SendToTcpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len, int tcp_thread_idx) = 0;

    virtual int SendToHttpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                 size_t msg_body_len, int http_thread_idx) = 0;

    virtual int SendToUdpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len, int udp_thread_idx) = 0;
};
}

#endif // WORK_THREADS_INC_WORK_SCHEDULER_INTERFACE_H_
