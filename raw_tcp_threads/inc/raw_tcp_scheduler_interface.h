#ifndef RAW_TCP_THREADS_INC_RAW_TCP_SCHEDULER_INTERFACE_H_
#define RAW_TCP_THREADS_INC_RAW_TCP_SCHEDULER_INTERFACE_H_

#include <stddef.h>

struct ConnGUID;

namespace tcp
{
namespace raw
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

    /**
     * @brief 断开与客户端的连接，可以关闭其它io线程管理的客户端
     * @param conn_guid
     * @return
     */
    virtual int CloseClient(const ConnGUID* conn_guid) = 0;

    virtual int SendToGlobalThread(const ConnGUID* conn_guid, const void* data, size_t len) = 0;

    /**
     * @brief 发送给tcp thread，包括自己
     * @param tcp_thread_idx 为-1则由框架自行调度一个tcp线程
     * @return
     */
    virtual int SendToTCPThread(const ConnGUID* conn_guid, const void* data, size_t len, int tcp_thread_idx) = 0;

    /**
     * @brief
     * @param work_thread_idx 为-1则由框架自行调度一个work线程
     * @return
     */
    virtual int SendToWorkThread(const ConnGUID* conn_guid, const void* data, size_t len, int work_thread_idx) = 0;
};
}
}

#endif // RAW_TCP_THREADS_INC_RAW_TCP_SCHEDULER_INTERFACE_H_
