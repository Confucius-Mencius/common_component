#ifndef CLIENT_CENTER_MGR_INC_HTTP_CLIENT_CENTER_INTERFACE_H_
#define CLIENT_CENTER_MGR_INC_HTTP_CLIENT_CENTER_INTERFACE_H_

#include "peer_sink_interface.h"

struct event_base;
class TimerAxisInterface;
class TransCenterInterface;

namespace http
{
struct ClientCenterCtx
{
    struct event_base* thread_ev_base;
    TimerAxisInterface* timer_axis;
    TransCenterInterface* trans_center;
    int http_conn_timeout;
    int http_conn_max_retry;

    ClientCenterCtx()
    {
        thread_ev_base = NULL;
        timer_axis = NULL;
        trans_center = NULL;
        http_conn_timeout = 0;
        http_conn_max_retry = 0;
    }
};

class ClientInterface
{
public:
    virtual ~ClientInterface()
    {
    }

    virtual const Peer& GetPeer() const = 0;

    // TODO head方法
    virtual TransId Head(const HeadParams& params, const base::AsyncCtx* async_ctx) = 0;

    virtual TransId Get(const http::GetParams& params, const base::AsyncCtx* async_ctx) = 0;

    virtual TransId Post(const PostParams& params, const base::AsyncCtx* async_ctx) = 0;
};

class ClientCenterInterface
{
public:
    virtual ~ClientCenterInterface()
    {
    }

    // TODO 现在Create和Get的时候都是把http和https都创建出来，后面可以优化一下，传一个参数进去，只创建一种
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

#endif // CLIENT_CENTER_MGR_INC_HTTP_CLIENT_CENTER_INTERFACE_H_
