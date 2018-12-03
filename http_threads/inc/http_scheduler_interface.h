#ifndef HTTP_THREADS_INC_HTTP_SCHEDULER_INTERFACE_H_
#define HTTP_THREADS_INC_HTTP_SCHEDULER_INTERFACE_H_

#include "scheduler_interface.h"

namespace http
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
     * @param evhttp_req
     * @param http_code
     * @param http_header_map
     * @param content
     * @param content_len
     * @return
     * @attention 只能发给本线程自己管理的连接
     */
    virtual int SendToClient(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, int http_code,
                             const KeyValMap* http_header_map, const char* content, size_t content_len) = 0;

    // http线程与其它线程直接的通信较特殊，返回值为事务ID，可以注册回调

    /**
     *
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param work_thread_idx
     * @param async_ctx
     * @return
     */
    virtual TransId SendToWorkThread(const ConnGuid* conn_guid, const MsgHead& msg_head,
                                     const void* msg_body, size_t msg_body_len, int work_thread_idx,
                                     const base::AsyncCtx* async_ctx) = 0;

    /**
     *
     * @param conn_guid
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param async_ctx
     * @return
     */
    virtual TransId SendToGlobalThread(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                                       size_t msg_body_len, const base::AsyncCtx* async_ctx) = 0;
};
}

#endif // HTTP_THREADS_INC_HTTP_SCHEDULER_INTERFACE_H_
