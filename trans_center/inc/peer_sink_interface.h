#ifndef TRANS_CENTER_INC_PEER_SINK_INTERFACE_H_
#define TRANS_CENTER_INC_PEER_SINK_INTERFACE_H_

#include "peer.h"
#include "proto_msg.h"
#include "seq_num.h"

// 事务ID
typedef int32_t TransID; /**< TransID类型 */
typedef I32SeqNum TransIDSeq; /**< TransIDSeq类型 */

#define INVALID_TRANS_ID INVALID_SEQ_NUM

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
    virtual void OnConnected(TransID trans_id, const Peer& peer, const void* data, size_t len) = 0;

    /**
     * @brief OnClosed 对端关闭了连接(TCP,HTTP)
     * @param trans_id
     * @param peer
     * @param data
     * @param len
     */
    virtual void OnClosed(TransID trans_id, const Peer& peer, const void* data, size_t len) = 0;

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
                           const void* msg_body, size_t msg_body_len, const void* data, size_t len) = 0;

    /**
     * @brief OnTimeout 超时对端未响应。threads, proto tcp
     * @param trans_id
     * @param peer
     * @param data
     * @param len
     */
    virtual void OnTimeout(TransID trans_id, const Peer& peer, const void* data, size_t len) = 0;
};

struct AsyncCtx
{
    int total_retries; // 重试次数，必须>=0
    int timeout_sec; // 需要回复的请求应该设置这个超时时间为>0；不需要回复或者需要回复但不管超时的时候为-1即可
    PeerSinkInterface* sink;
    const void* data;
    size_t len;

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
