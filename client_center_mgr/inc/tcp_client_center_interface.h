#ifndef CLIENT_CENTER_MGR_INC_TCP_CLIENT_CENTER_INTERFACE_H_
#define CLIENT_CENTER_MGR_INC_TCP_CLIENT_CENTER_INTERFACE_H_

#include "peer_sink_interface.h"

struct event_base;
class MsgCodecInterface;
class TimerAxisInterface;
class TransCenterInterface;

namespace tcp
{
struct ClientCenterCtx
{
    struct event_base* thread_ev_base;
    MsgCodecInterface* msg_codec;
    TimerAxisInterface* timer_axis;
    TransCenterInterface* trans_center;
    size_t max_msg_body_len;
    struct timeval reconnect_interval; /**< 重连间隔 */

    ClientCenterCtx()
    {
        thread_ev_base = NULL;
        msg_codec = NULL;
        timer_axis = NULL;
        trans_center = NULL;
        max_msg_body_len = 0;
        reconnect_interval.tv_sec = reconnect_interval.tv_usec = 0;
    }
};

class NfySinkInterface
{
public:
    virtual ~NfySinkInterface()
    {
    }

    virtual void OnRecvNfy(const Peer& peer, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len) = 0;
};

class ClientInterface
{
public:
    virtual ~ClientInterface()
    {
    }

    /**
     * @brief 获取连接key，key是标识该连接的关键字
     * @return
     */
    virtual const Peer& GetPeer() const = 0;

    virtual int AddNfySink(NfySinkInterface* sink) = 0;
    virtual void RemoveNfySink(NfySinkInterface* sink) = 0;

    /**
     * @brief 检查是否处于连接状态
     * @return
     */
    virtual bool IsConnected() const = 0;

    /**
     * @brief 通过该TCP连接发送消息给对端
     * @return
     */
    virtual TransId Send(const MsgHead& msg_head, const void* msg_body, size_t msg_body_len,
                         const base::AsyncCtx* async_ctx) = 0;

    /**
     * @brief 手动关闭连接，并释放对象
     */
    virtual void Close() = 0;
};

class ClientCenterInterface
{
public:
    virtual ~ClientCenterInterface()
    {
    }

    virtual ClientInterface* CreateClient(const Peer& peer) = 0;

    /**
     *
     * @param peer
     * @attention 其中会释放client对象
     */
    virtual void RemoveClient(const Peer& peer) = 0;

    virtual ClientInterface* GetClient(const Peer& peer) = 0;
};
}

#endif // CLIENT_CENTER_MGR_INC_TCP_CLIENT_CENTER_INTERFACE_H_
