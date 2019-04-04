#ifndef TRANS_CENTER_SRC_TRANS_CENTER_H_
#define TRANS_CENTER_SRC_TRANS_CENTER_H_

#include <set>
#include "hash_container.h"
#include "need_rsp_msg_mgr.h"

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
    TransID RecordTransCtx(const TransCtx* ctx) override;
    void CancelTrans(TransID trans_id) override;
    TransCtx* GetTransCtx(TransID trans_id) const override;
    void OnConnected(const Peer& peer) override;
    void OnClosed(const Peer& peer) override;
    void OnRecvRsp(TransID trans_id, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len) override;

public:
    void RemoveTrans(TransID trans_id);
    void OnTimeout(TransID trans_id);

private:
    TransCenterCtx trans_center_ctx_;

    typedef std::set<TransCenterSinkInterface*> TransCenterSinkSet;
    TransCenterSinkSet trans_center_sink_set_;

    TransIDSeq trans_id_seq_;

    typedef __hash_map<TransID, Trans*> TransHashMap;
    TransHashMap trans_hash_map_;

    typedef std::set<TransID> TransIDSet;
    typedef std::map<Peer, TransIDSet> PeerTransIDMap;
    PeerTransIDMap peer_trans_id_map_;

    NeedRspMsgMgr need_rsp_msg_mgr_;
};
}

#endif // TRANS_CENTER_SRC_TRANS_CENTER_H_
