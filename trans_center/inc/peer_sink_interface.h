#ifndef TRANS_CENTER_INC_PEER_SINK_INTERFACE_H_
#define TRANS_CENTER_INC_PEER_SINK_INTERFACE_H_

#include "common_define.h"
#include "msg_define.h"

namespace http
{
struct GetParams
{
    bool https; // 是否为https
    const char* uri; // 可以是未做encode的原始uri，也可以是做了encode后的uri
    int uri_len; // 当uri是以\0结尾的字符串时，uri_len可以传-1
    bool need_encode; // 是否需要对uri做encode
    const KeyValMap* header_map;

    GetParams()
    {
        https = false;
        uri = NULL;
        uri_len = 0;
        need_encode = false;
        header_map = NULL;
    }
};

struct PostParams
{
    GetParams get_params;
    const void* data;
    size_t data_len;

    PostParams() : get_params()
    {
        data = NULL;
        data_len = 0;
    }
};

struct HeadParams
{

};

struct Rsp
{
    bool https;
    int http_code;
    const char* status_line;
    const KeyValMap* header_map;
    const char* rsp_body;
    size_t rsp_body_len;

    Rsp()
    {
        https = false;
        http_code = 0;
        status_line = NULL;
        header_map = NULL;
        rsp_body = NULL;
        rsp_body_len = 0;
    }
};
}

/**
 * @brief 与对端通信的回调接口
 */
class PeerSinkInterface
{
public:
    virtual ~PeerSinkInterface()
    {
    }

    /**
     * @brief 连上了对端(TCP)
     * @param trans_id
     * @param async_data
     * @param async_data_len
     */
    virtual void OnConnected(TransId trans_id, const Peer& peer, const void* async_data, size_t async_data_len) = 0;

    /**
     * @brief 对端关闭了连接(TCP,HTTP)
     * @param trans_id
     * @param async_data
     * @param async_data_len
     */
    virtual void OnClosed(TransId trans_id, const Peer& peer, const void* async_data, size_t async_data_len) = 0;

    // tcp/udp/thread rsp
    /**
     * @brief tcp,udp,thread rsp
     * @param trans_id
     * @param peer
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param async_data
     * @param async_data_len
     */
    virtual void OnRecvRsp(TransId trans_id, const Peer& peer, const MsgHead& msg_head, const void* msg_body,
                           size_t msg_body_len, const void* async_data, size_t async_data_len) = 0;

    /**
     * @brief http/https rsp
     * @param trans_id
     * @param http_rsp
     * @param async_data
     * @param async_data_len
     */
    virtual void OnRecvRsp(TransId trans_id, const Peer& peer, const http::Rsp* http_rsp, const void* async_data,
                           size_t async_data_len) = 0;


    /**
     * @brief 超时对端未响应(TCP,HTTP/HTTPS,UDP,THREAD)都适用
     * @param trans_id
     * @param async_data
     * @param async_data_len
     */
    virtual void OnTimeout(TransId trans_id, const Peer& peer, const void* async_data, size_t async_data_len) = 0;
};

namespace base
{
/**
 * @param total_retry 重试次数，必须>=0
 * @param timeout_sec 等待对端回复的超时时间，单位：秒
 * @param sink
 * @param async_data
 * @param async_data_len
 */
struct AsyncCtx
{
    int total_retry;
    int timeout_sec; // 需要回复的请求应该设置这个超时时间为>0；不需要回复或者需要回复但不管超时的时候为-1即可
    PeerSinkInterface* sink;
    const void* async_data;
    size_t async_data_len;

    AsyncCtx()
    {
        total_retry = 0;
        timeout_sec = -1;
        sink = NULL;
        async_data = NULL;
        async_data_len = 0;
    }
};
}

#endif // TRANS_CENTER_INC_PEER_SINK_INTERFACE_H_
