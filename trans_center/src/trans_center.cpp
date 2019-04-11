#include "trans_center.h"
#include "version.h"

namespace trans_center
{
TransCenter::TransCenter() : trans_center_ctx_(), trans_center_sink_set_(), trans_id_seq_(), trans_hash_map_(),
    peer_trans_id_map_(), need_rsp_msg_mgr_()
{
}

TransCenter::~TransCenter()
{
}

const char* TransCenter::GetVersion() const
{
    return TRANS_CENTER_TRANS_CENTER_VERSION;
}

const char* TransCenter::GetLastErrMsg() const
{
    return nullptr;
}

void TransCenter::Release()
{
    TransHashMap tmp_trans_map = trans_hash_map_;

    for (TransHashMap::iterator it = tmp_trans_map.begin(); it != tmp_trans_map.end(); ++it)
    {
        CancelTrans(it->first);
    }

    trans_hash_map_.clear();
    trans_center_sink_set_.clear();
    peer_trans_id_map_.clear();

    delete this;
}

int TransCenter::Initialize(const void* ctx)
{
    if (nullptr == ctx)
    {
        return -1;
    }

    trans_center_ctx_ = *(static_cast<const TransCenterCtx*>(ctx));
    need_rsp_msg_mgr_.SetTransCenter(this);

    if (need_rsp_msg_mgr_.Initialize(trans_center_ctx_.timer_axis, { trans_center_ctx_.need_rsp_msg_check_interval, 0 }) != 0)
    {
        return -1;
    }

    return 0;
}

void TransCenter::Finalize()
{
    need_rsp_msg_mgr_.Finalize();
}

int TransCenter::Activate()
{
    if (need_rsp_msg_mgr_.Activate() != 0)
    {
        return -1;
    }

    return 0;
}

void TransCenter::Freeze()
{
    need_rsp_msg_mgr_.Freeze();
}

int TransCenter::AddSink(TransCenterSinkInterface* sink)
{
    if (trans_center_sink_set_.find(sink) != trans_center_sink_set_.end())
    {
        LOG_ERROR("trans center sink already exist: " << sink);
        return -1;
    }

    if (!trans_center_sink_set_.insert(sink).second)
    {
        const int err = errno;
        LOG_ERROR("failed to insert to set, trans center sink: " << sink
                  << ", errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    LOG_TRACE("insert to set ok, trans center sink:: " << sink);
    return 0;
}

void TransCenter::RemoveSink(TransCenterSinkInterface* sink)
{
    TransCenterSinkSet::iterator it = trans_center_sink_set_.find(sink);
    if (it == trans_center_sink_set_.end())
    {
        LOG_WARN("failed to find trans center sink: " << sink);
        return;
    }

    trans_center_sink_set_.erase(it);
}

TransID TransCenter::RecordTransCtx(const TransCtx* ctx)
{
    if (nullptr == ctx)
    {
        return INVALID_TRANS_ID;
    }

    Trans* trans = Trans::Create(ctx);
    if (nullptr == trans)
    {
        return INVALID_TRANS_ID;
    }

    int ret = -1;
    TransID trans_id = INVALID_TRANS_ID;

    do
    {
        trans_id = trans_id_seq_.Alloc();
        if (INVALID_TRANS_ID == trans_id)
        {
            LOG_ERROR("failed to alloc trans id");
            break;
        }

        if (!trans_hash_map_.insert(TransHashMap::value_type(trans_id, trans)).second)
        {
            LOG_ERROR("failed to insert to map, trans id: " << trans_id);
            break;
        }

        if (ctx->sink != nullptr && ctx->timeout_sec > 0)
        {
            need_rsp_msg_mgr_.UpsertRecord(trans_id, trans, ctx->timeout_sec);
        }

        peer_trans_id_map_[ctx->peer].insert(trans_id);

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (INVALID_TRANS_ID == trans_id)
        {
            trans->Release();
        }
        else
        {
            CancelTrans(trans_id);
        }

        return INVALID_TRANS_ID;
    }

    if (nullptr == ctx->sink)
    {
        CancelTrans(trans_id); // 不需要回复，释放掉trans id
    }

    return trans_id;
}

void TransCenter::CancelTrans(TransID trans_id)
{
    TransHashMap::iterator it = trans_hash_map_.find(trans_id);
    if (it == trans_hash_map_.end())
    {
        return;
    }

    if (need_rsp_msg_mgr_.RecordExist(trans_id))
    {
        need_rsp_msg_mgr_.RemoveRecord(trans_id);
    }

    RemoveTrans(trans_id);
}

TransCtx* TransCenter::GetTransCtx(TransID trans_id) const
{
    TransHashMap::const_iterator it = trans_hash_map_.find(trans_id);
    if (it == trans_hash_map_.cend())
    {
        return nullptr;
    }

    return const_cast<TransCtx*>(it->second->GetCtx());
}

void TransCenter::OnConnected(const Peer& peer)
{
    PeerTransIDMap::iterator it_peer_trans_id = peer_trans_id_map_.find(peer);
    if (it_peer_trans_id == peer_trans_id_map_.end())
    {
        return;
    }

    TransIDSet& trans_id_set = it_peer_trans_id->second;
    for (TransIDSet::iterator it_trans_id = trans_id_set.begin(); it_trans_id != trans_id_set.end(); ++it_trans_id)
    {
        TransHashMap::iterator it_trans = trans_hash_map_.find(*it_trans_id);
        if (it_trans == trans_hash_map_.end())
        {
            continue;
        }

        TransCtx* trans_ctx = const_cast<TransCtx*>(it_trans->second->GetCtx());
        if (trans_ctx->sink != nullptr)
        {
            trans_ctx->sink->OnConnected(*it_trans_id, peer, trans_ctx->data, trans_ctx->len);
        }
    }
}

void TransCenter::OnClosed(const Peer& peer)
{
    PeerTransIDMap::iterator it_peer_trans_id = peer_trans_id_map_.find(peer);
    if (it_peer_trans_id == peer_trans_id_map_.end())
    {
        return;
    }

    TransIDSet& trans_id_set = it_peer_trans_id->second;
    for (TransIDSet::iterator it_trans_id = trans_id_set.begin(); it_trans_id != trans_id_set.end(); ++it_trans_id)
    {
        TransHashMap::iterator it_trans = trans_hash_map_.find(*it_trans_id);
        if (it_trans == trans_hash_map_.end())
        {
            continue;
        }

        TransCtx* trans_ctx = const_cast<TransCtx*>(it_trans->second->GetCtx());
        if (trans_ctx->sink != nullptr)
        {
            trans_ctx->sink->OnClosed(*it_trans_id, peer, trans_ctx->data, trans_ctx->len);
        }
    }
}

void TransCenter::OnRecvRsp(TransID trans_id, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
{
    TransHashMap::iterator it = trans_hash_map_.find(trans_id);
    if (it != trans_hash_map_.end())
    {
        TransCtx* trans_ctx = const_cast<TransCtx*>(it->second->GetCtx());
        if (trans_ctx->sink != nullptr)
        {
            trans_ctx->sink->OnRecvRsp(trans_id, trans_ctx->peer, msg_head, msg_body, msg_body_len,
                                       trans_ctx->data, trans_ctx->len);
        }
    }

    CancelTrans(trans_id);
}

void TransCenter::RemoveTrans(TransID trans_id)
{
    TransHashMap::iterator it = trans_hash_map_.find(trans_id);
    if (it == trans_hash_map_.end())
    {
        return;
    }

    peer_trans_id_map_[it->second->GetCtx()->peer].erase(it->first);
    trans_id_seq_.Free(it->first);
    it->second->Release();
    trans_hash_map_.erase(it);
}

void TransCenter::OnTimeout(TransID trans_id)
{
    TransHashMap::iterator it = trans_hash_map_.find(trans_id);
    if (it != trans_hash_map_.end())
    {
        TransCtx* trans_ctx = const_cast<TransCtx*>(it->second->GetCtx());
        if (trans_ctx->sink != nullptr)
        {
            trans_ctx->sink->OnTimeout(trans_id, trans_ctx->peer, trans_ctx->data, trans_ctx->len);
        }
    }

    for (TransCenterSinkSet::iterator it = trans_center_sink_set_.begin(); it != trans_center_sink_set_.end(); ++it)
    {
        (*it)->OnTimeout(trans_id);
    }

    RemoveTrans(trans_id);
}
}
