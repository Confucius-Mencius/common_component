#ifndef WS_THREADS_INC_HTTP_SCHEDULER_INTERFACE_H_
#define WS_THREADS_INC_HTTP_SCHEDULER_INTERFACE_H_

#include "conn_define.h"

namespace ws
{
namespace http
{
class SchedulerInterface
{
public:
    virtual ~SchedulerInterface()
    {
    }

    /**
     * @brief 可以发往其它io线程管理的客户端
     * @return
     */
    virtual int SendToClient(const ConnGUID* conn_guid, const void* data, size_t len) = 0;

    /**
     * @brief 断开与客户端的连接，可以关闭其它io线程管理的客户端
     * @param conn_guid
     * @return
     */
    virtual int CloseClient(const ConnGUID* conn_guid) = 0;

    // 自己这组线程，下一级线程组，global线程组

    /**
     * @brief 发送给ws thread，包括自己
     * @param ws_thread_idx 为-1则由框架自行调度一个ws线程
     * @return
     */
    virtual int SendToWSThread(const ConnGUID* conn_guid, const void* data, size_t len, int ws_thread_idx) = 0;

    /**
     * @brief
     * @param work_thread_idx 为-1则由框架自行调度一个work线程
     * @return
     */
    virtual int SendToWorkThread(const ConnGUID* conn_guid, const void* data, size_t len, int work_thread_idx) = 0;

    virtual int SendToGlobalThread(const ConnGUID* conn_guid, const void* data, size_t len) = 0;
};
}
}

#endif // WS_THREADS_INC_HTTP_SCHEDULER_INTERFACE_H_
