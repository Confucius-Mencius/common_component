#ifndef TRANS_CENTER_SRC_NEED_REPLY_MSG_MGR_H_
#define TRANS_CENTER_SRC_NEED_REPLY_MSG_MGR_H_

#include "record_timeout_mgr.h"
#include "trans_define.h"

namespace trans_center
{
class TransCenter;

class NeedReplyMsgMgr : public RecordTimeoutMgr<TransId, std::hash<TransId>, Trans*>
{
public:
    NeedReplyMsgMgr();
    virtual ~NeedReplyMsgMgr();

    void SetTransCenter(TransCenter* trans_center)
    {
        trans_center_ = trans_center;
    }

protected:
    ///////////////////////// RecordTimeoutMgr<TransId, std::hash<TransId>, Trans*> /////////////////////////
    void OnTimeout(const TransId& k, Trans* const& v, int timeout_sec) override;

private:
    TransCenter* trans_center_;
};
}

#endif // TRANS_CENTER_SRC_NEED_REPLY_MSG_MGR_H_
