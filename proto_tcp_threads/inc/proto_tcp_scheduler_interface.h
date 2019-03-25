#ifndef PROTO_TCP_THREADS_INC_PROTO_TCP_SCHEDULER_INTERFACE_H_
#define PROTO_TCP_THREADS_INC_PROTO_TCP_SCHEDULER_INTERFACE_H_

#include "conn.h"
#include "proto_msg.h"

namespace tcp
{
namespace proto
{
class SchedulerInterface
{
public:
    virtual ~SchedulerInterface()
    {
    }

//    virtual int SendToClient(const ConnGUID* conn_guid, const proto::MsgHead& msg_head, const void* msg_body,
//                             size_t msg_body_len) = 0;

//    // send序列号好的数据
//    virtual int SendToClient(const ConnGUID* conn_guid, const void* data, size_t len) = 0;

//    /**
//     * @brief 断开与客户端的连接，可以关闭其它io线程管理的客户端
//     * @param conn_guid
//     * @return
//     */
//    virtual int CloseClient(const ConnGUID* conn_guid) = 0;

//    // 自己这组线程，下一级线程组

//    /**
//     * @brief 发送给tcp thread，包括自己
//     * @param tcp_thread_idx 为-1则由框架自行调度
//     * @return
//     */
//    virtual int SendToTCPThread(const ConnGUID* conn_guid, const proto::MsgHead& msg_head, const void* msg_body,
//                                size_t msg_body_len, int tcp_thread_idx) = 0;

//    /**
//     * @brief
//     * @param work_thread_idx 为-1则由框架自行调度
//     * @return
//     */
//    virtual int SendToWorkThread(const ConnGUID* conn_guid, const proto::MsgHead& msg_head, const void* msg_body,
//                                 size_t msg_body_len, int work_thread_idx) = 0;

//    virtual int SendToGlobalThread(const ConnGUID* conn_guid, const proto::MsgHead& msg_head, const void* msg_body,
//                                   size_t msg_body_len) = 0;
};
}
}

#endif // PROTO_TCP_THREADS_INC_PROTO_TCP_SCHEDULER_INTERFACE_H_
