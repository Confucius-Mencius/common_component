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

    /**
     * @brief send to client，可以发往其它io线程管理的客户端
     * @return
     */
    virtual int SendToClient(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                             const void* msg_body, size_t msg_body_len) = 0;

    /**
     * @brief send to client，send序列化好的数据，可以发往其它io线程管理的客户端
     * @return
     */
    virtual int SendToClient(const ConnGUID* conn_guid, const void* data, size_t len) = 0;

    /**
     * @brief 断开与客户端的连接，可以关闭其它io线程管理的客户端
     * @param conn_guid
     * @return
     */
    virtual int CloseClient(const ConnGUID* conn_guid) = 0;

    virtual int SendToGlobalThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                   const void* msg_body, size_t msg_body_len) = 0;

    /**
     * @brief SendToWorkThread
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param work_thread_idx 为-1则由框架自行调度一个work线程
     * @return
     */
    virtual int SendToWorkThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                 const void* msg_body, size_t msg_body_len, int work_thread_idx) = 0;

    /**
     * @brief SendToBurdenThread
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param burden_thread_idx 为-1则由框架自行调度一个burden线程
     * @return
     */
    virtual int SendToBurdenThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                   const void* msg_body, size_t msg_body_len, int burden_thread_idx) = 0;

    /**
     * @brief SendToTCPThread
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param tcp_thread_idx 为-1则由框架自行调度一个tcp线程
     * @return
     */
    virtual int SendToTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                const void* msg_body, size_t msg_body_len, int tcp_thread_idx) = 0;

    /**
     * @brief SendToProtoTCPThread
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param tcp_thread_idx 为-1则由框架自行调度一个proto tcp线程
     * @return
     */
    virtual int SendToProtoTCPThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                     const void* msg_body, size_t msg_body_len, int proto_tcp_thread_idx) = 0;

    /**
     * @brief SendToHTTPWSThread
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param http_ws_thread_idx 为-1则由框架自行调度一个http-ws tcp线程
     * @return
     */
    virtual int SendToHTTPWSThread(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head,
                                   const void* msg_body, size_t msg_body_len, int http_ws_thread_idx) = 0;
};
}

#endif // WORK_THREADS_INC_WORK_SCHEDULER_INTERFACE_H_
