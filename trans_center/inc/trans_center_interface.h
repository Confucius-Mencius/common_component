#ifndef TRANS_CENTER_INC_TRANS_CENTER_INTERFACE_H_
#define TRANS_CENTER_INC_TRANS_CENTER_INTERFACE_H_

#include "module_interface.h"
#include "peer_sink_interface.h"

class TimerAxisInterface;

struct TransCtx
{
    Peer peer;
    int timeout_sec;
    Passback passback;
    PeerSinkInterface* sink;
    char* async_data;
    size_t async_data_len;

    TransCtx() : peer()
    {
        timeout_sec = 0;
        passback = -1;
        sink = NULL;
        async_data = NULL;
        async_data_len = 0;
    }
};

struct TransCenterCtx
{
    TimerAxisInterface* timer_axis;
    int need_reply_msg_check_interval;

    TransCenterCtx()
    {
        timer_axis = NULL;
        need_reply_msg_check_interval = 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
// 接口设计：
// 1，connect,close,recv收到对端的数据，这些都是外部发起的，trans center只能提供接口，
// 在外部发现connected、closed、recv收到对端的数据时调用。
// 2，超时未回复是trans center内部管理的，超时的时候需要通知外部。
////////////////////////////////////////////////////////////////////////////////

class TransCenterSinkInterface
{
public:
    virtual ~TransCenterSinkInterface()
    {
    }

    virtual void OnTimeout(TransId trans_id) = 0; // 对端超时
};

class TransCenterInterface : public ModuleInterface
{
public:
    virtual ~TransCenterInterface()
    {
    }

    virtual int AddSink(TransCenterSinkInterface* sink) = 0;
    virtual void RemoveSink(TransCenterSinkInterface* sink) = 0;

    virtual TransId RecordTransCtx(const TransCtx* ctx) = 0;
    virtual void CancelTrans(TransId trans_id) = 0;
    virtual TransCtx* GetTransCtx(TransId trans_id) const = 0;

    virtual void OnConnected(const Peer& peer) = 0;
    virtual void OnClosed(const Peer& peer) = 0;

    virtual void OnRecvRsp(TransId trans_id, const MsgHead& msg_head, const char* msg_body, size_t msg_body_len) = 0;
    virtual void OnRecvHttpRsp(TransId trans_id, const http::Rsp* http_rsp) = 0;
};

#endif // TRANS_CENTER_INC_TRANS_CENTER_INTERFACE_H_
