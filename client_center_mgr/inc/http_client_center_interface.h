#ifndef CLIENT_CENTER_MGR_INC_HTTP_CLIENT_CENTER_INTERFACE_H_
#define CLIENT_CENTER_MGR_INC_HTTP_CLIENT_CENTER_INTERFACE_H_

#include "peer_sink_interface.h"

struct event_base;
class TimerAxisInterface;
class TransCenterInterface;

namespace http
{
struct GetParams
{
    bool https; // 是否为https
    const char* uri; // 可以是未做encode的原始uri，也可以是做了encode后的uri
    int uri_len; // 当uri是以\0结尾的字符串时，uri_len可以传-1
    bool need_encode; // 是否需要对uri做encode
    const HeaderMap* headers;

    GetParams()
    {
        https = false;
        uri = nullptr;
        uri_len = 0;
        need_encode = false;
        headers = nullptr;
    }
};

struct PostParams
{
    GetParams get_params;
    const void* data;
    size_t len;

    PostParams() : get_params()
    {
        data = nullptr;
        len = 0;
    }
};

struct ClientCenterCtx
{
    struct event_base* thread_ev_base;
    TimerAxisInterface* timer_axis;
    TransCenterInterface* trans_center;
    int http_conn_timeout;
    int http_conn_max_retry;

    ClientCenterCtx()
    {
        thread_ev_base = nullptr;
        timer_axis = nullptr;
        trans_center = nullptr;
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

    virtual TransID Get(const GetParams& params, const AsyncCtx* async_ctx) = 0;

    virtual TransID Post(const PostParams& params, const AsyncCtx* async_ctx) = 0;
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
