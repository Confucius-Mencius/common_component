#ifndef TRANS_CENTER_SRC_TRANS_CENTER_H_
#define TRANS_CENTER_SRC_TRANS_CENTER_H_

#include <set>
#include "hash_container.h"
#include "need_reply_msg_mgr.h"

namespace trans_center
{
class TransCenter : public TransCenterInterface
{
public:
    TransCenter();
    virtual ~TransCenter();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// TransCenterInterface /////////////////////////
    int AddSink(TransCenterSinkInterface* sink) override;
    void RemoveSink(TransCenterSinkInterface* sink) override;
    TransId RecordTransCtx(const TransCtx* ctx) override;
    void CancelTrans(TransId trans_id) override;
    TransCtx* GetTransCtx(TransId trans_id) const override;
    void OnConnected(const Peer& peer) override;
    void OnClosed(const Peer& peer) override;
    void OnRecvRsp(TransId trans_id, const MsgHead& msg_head, const char* msg_body, size_t msg_body_len) override;
    void OnRecvHttpRsp(TransId trans_id, const http::Rsp* http_rsp) override;

public:
    void RemoveLogic(TransId trans_id);
    void TimeoutLogic(TransId trans_id);

private:
    TransCenterCtx trans_center_ctx_;

    typedef std::set<TransCenterSinkInterface*> TransCenterSinkSet;
    TransCenterSinkSet trans_center_sink_set_;

    TransIdSeq trans_id_seq_;

    typedef __hash_map<TransId, Trans*> TransHashMap;
    TransHashMap trans_hash_map_;

    typedef std::set<TransId> TransIdSet;
    typedef std::map<Peer, TransIdSet> PeerTransIdMap;
    PeerTransIdMap peer_trans_id_map_;

    NeedReplyMsgMgr need_reply_msg_mgr_;
};
}

#endif // TRANS_CENTER_SRC_TRANS_CENTER_H_
