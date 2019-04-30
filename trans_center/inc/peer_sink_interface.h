#ifndef TRANS_CENTER_INC_PEER_SINK_INTERFACE_H_
#define TRANS_CENTER_INC_PEER_SINK_INTERFACE_H_

#include <map>
#include "peer.h"
#include "proto_msg.h"
#include "seq_num.h"

// 事务ID
typedef int32_t TransID; /**< TransID类型 */
typedef I32SeqNum TransIDSeq; /**< TransIDSeq类型 */

#define INVALID_TRANS_ID INVALID_SEQ_NUM

namespace http
{
typedef std::map<std::string, std::string> HeaderMap;

struct Rsp
{
    bool https; // 是否为https
    int status_code;
    const char* status_line;
    const HeaderMap* headers;
    const char* rsp_body;
    size_t rsp_body_len;

    Rsp()
    {
        https = false;
        status_code = -1;
        status_line = nullptr;
        headers = nullptr;
        rsp_body = nullptr;
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
     * @brief OnConnected 连上了对端(TCP)
     * @param trans_id
     * @param peer
     * @param data
     * @param len
     */
    virtual void OnConnected(TransID trans_id, const Peer& peer, void* data, size_t len) {}

    /**
     * @brief OnClosed 对端关闭了连接(TCP,HTTP)
     * @param trans_id
     * @param peer
     * @param data
     * @param len
     */
    virtual void OnClosed(TransID trans_id, const Peer& peer, void* data, size_t len) {}

    /**
     * @brief OnRecvRsp threads, proto tcp
     * @param trans_id
     * @param peer
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param data
     * @param len
     */
    virtual void OnRecvRsp(TransID trans_id, const Peer& peer, const ::proto::MsgHead& msg_head,
                           const void* msg_body, size_t msg_body_len, void* data, size_t len) {}

    virtual void OnRecvHTTPRsp(TransID trans_id, const Peer& peer, const http::Rsp* http_rsp, void* data, size_t len) {}

    /**
     * @brief OnTimeout 超时对端未响应。threads, proto tcp
     * @param trans_id
     * @param peer
     * @param data
     * @param len
     */
    virtual void OnTimeout(TransID trans_id, const Peer& peer, void* data, size_t len) {}
};

struct AsyncCtx
{
    int total_retries; // 发送失败时重试次数，必须>=0。http请求不使用该字段
    int timeout_sec; // 需要回复的请求应该设置这个超时时间为>0；不需要回复或者需要回复但不管超时的时候为-1即可。
    PeerSinkInterface* sink;
    void* data; // 回调时使用的异步数据，可以是一个对象的指针，也可以是一块内存区
    size_t len; // 如果data是一个对象的指针，len为0；如果时一块内存区，len为内存区的长度

    AsyncCtx()
    {
        total_retries = 0;
        timeout_sec = -1;
        sink = nullptr;
        data = nullptr;
        len = 0;
    }
};

#endif // TRANS_CENTER_INC_PEER_SINK_INTERFACE_H_
