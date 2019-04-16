#ifndef UDP_THREADS_INC_UDP_SCHEDULER_INTERFACE_H_
#define UDP_THREADS_INC_UDP_SCHEDULER_INTERFACE_H_

#include <stddef.h>

struct ConnGUID;

namespace proto
{
struct MsgHead;
}

namespace udp
{
class SchedulerInterface
{
public:
    virtual ~SchedulerInterface()
    {
    }


    /**
     * @brief send to client，可以发往其它io线程管理的客户端
     * @return
     */
    virtual int SendToClient(const ConnGUID* conn_guid, const void* data, size_t len) = 0;

    virtual int SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                   const void* msg_body, size_t msg_body_len) = 0;

    virtual int SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                 const void* msg_body, size_t msg_body_len, int work_thread_idx) = 0;

    /**
     * @brief 发送给udp thread，包括自己
     * @param udp_thread_idx 为-1则由框架自行调度一个udp线程
     * @return
     */
    virtual int SendToUDPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                const void* msg_body, size_t msg_body_len, int udp_thread_idx) = 0;
};
}

#endif // UDP_THREADS_INC_UDP_SCHEDULER_INTERFACE_H_
