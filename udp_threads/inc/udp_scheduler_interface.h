#ifndef UDP_THREADS_INC_UDP_SCHEDULER_INTERFACE_H_
#define UDP_THREADS_INC_UDP_SCHEDULER_INTERFACE_H_

#include "scheduler_interface.h"

namespace udp
{
class SchedulerInterface : public base::SchedulerInterface
{
public:
    virtual ~SchedulerInterface()
    {
    }

    /**
     * @brief send msg to client，可以发往其它io线程管理的客户端
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @return
     */
    virtual int SendToClient(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                             size_t msg_body_len) = 0;

    /**
     * @brief 断开与客户端的连接，可以关闭其它io线程管理的客户端
     * @param conn_guid
     * @return
     */
    virtual int CloseClient(const ConnGuid* conn_guid) = 0;

    // 自己这组线程，下一级线程组

    /**
     * @brief 发送给udp thread，包括自己
     * @param udp_thread_idx 为-1则由框架自行调度
     * @return
     */
    virtual int SendToUdpThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                size_t msg_body_len, int udp_thread_idx) = 0;

    /**
     * @brief
     * @param work_thread_idx 为-1则由框架自行调度
     * @return
     */
    virtual int SendToWorkThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                 size_t msg_body_len, int work_thread_idx) = 0;

    virtual int SendToGlobalThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                   size_t msg_body_len) = 0;
};
}

#endif // UDP_THREADS_INC_UDP_SCHEDULER_INTERFACE_H_
