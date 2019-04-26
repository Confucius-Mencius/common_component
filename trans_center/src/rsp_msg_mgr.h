#ifndef TRANS_CENTER_SRC_RSP_MSG_MGR_H_
#define TRANS_CENTER_SRC_RSP_MSG_MGR_H_

#include "record_timeout_mgr.h"
#include "trans.h"

namespace trans_center
{
class TransCenter;

class RspMsgMgr : public RecordTimeoutMgr<TransID, std::hash<TransID>, Trans*>
{
public:
    RspMsgMgr();
    virtual ~RspMsgMgr();

    void SetTransCenter(TransCenter* trans_center)
    {
        trans_center_ = trans_center;
    }

protected:
    ///////////////////////// RecordTimeoutMgr<TransId, std::hash<TransId>, Trans*> /////////////////////////
    void OnTimeout(const TransID& k, Trans* const& v, int timeout_sec) override;

private:
    TransCenter* trans_center_;
};
}

#endif // TRANS_CENTER_SRC_RSP_MSG_MGR_H_
