#ifndef THREAD_CENTER_INC_SCHEDULER_INTERFACE_H_
#define THREAD_CENTER_INC_SCHEDULER_INTERFACE_H_

#include "tcp_client_center_interface.h"
#include "http_client_center_interface.h"
#include "udp_client_center_interface.h"
#include "peer_sink_interface.h"

struct event_base;

namespace base
{
class SchedulerInterface
{
public:
    virtual ~SchedulerInterface()
    {
    }

    /**
     * @brief 获取所在thread的event base
     * @return
     */
    virtual struct event_base* GetThreadEvBase() const = 0;

    /**
     * @brief 获取所在thread的idx
     * @return
     */
    virtual int GetThreadIdx() const = 0;

    /**
     * @brief 发送tcp/udp消息给其它服务器
     * @param peer
     * @param params
     * @return
     */
    virtual TransId SendToServer(const Peer& peer, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len,
                                 const AsyncCtx* async_ctx) = 0;

    /**
     * @brief 向其它web服务器发起http get请求
     * @param peer
     * @param params
     * @return
     */
    virtual TransId HttpGet(const Peer& peer, const http::GetParams& params, const AsyncCtx* async_ctx) = 0;

    /**
     * @brief 向其它web服务器发起http post请求
     * @param peer
     * @param params
     * @return
     */
    virtual TransId HttpPost(const Peer& peer, const http::PostParams& params, const AsyncCtx* async_ctx) = 0;

    /**
     * @brief 向其它web服务器发起http head请求
     * @param peer
     * @param params
     * @return
     */
    virtual TransId HttpHead(const Peer& peer, const http::HeadParams& params, const AsyncCtx* async_ctx) = 0;

    /**
     * @brief 取消异步事务
     * @param trans_id
     */
    virtual void CancelTrans(TransId trans_id) = 0;
};
}

#endif // THREAD_CENTER_INC_SCHEDULER_INTERFACE_H_
