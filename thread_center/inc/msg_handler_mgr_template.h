#ifndef THREAD_CENTER_INC_MSG_HANDLER_MGR_TEMPLATE_H_
#define THREAD_CENTER_INC_MSG_HANDLER_MGR_TEMPLATE_H_

#include <vector>
#include "msg_handler_interface.h"

namespace base
{
template<typename MsgHandler>
class MsgHandlerMgrTemplate : public MsgHandlerMgrInterface
{
public:
    MsgHandlerMgrTemplate();
    virtual ~MsgHandlerMgrTemplate();

    virtual int Initialize(const void* ctx);
    virtual void Finalize();
    virtual int Activate();
    virtual void Freeze();

protected:
    virtual int InitializeMsgHandlerVec() = 0;
    virtual void FinalizeMsgHandlerVec() = 0;

protected:
    typedef std::vector<MsgHandler*> MsgHandlerVec;
    MsgHandlerVec msg_handler_vec_;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

template<typename MsgHandler>
MsgHandlerMgrTemplate<MsgHandler>::MsgHandlerMgrTemplate()
{
}

template<typename MsgHandler>
MsgHandlerMgrTemplate<MsgHandler>::~MsgHandlerMgrTemplate()
{
}

template<typename MsgHandler>
int MsgHandlerMgrTemplate<MsgHandler>::Initialize(const void* ctx)
{
    if (InitializeMsgHandlerVec() != 0)
    {
        return -1;
    }

    for (typename MsgHandlerVec::iterator it = msg_handler_vec_.begin(); it != msg_handler_vec_.end(); ++it)
    {
        MsgHandler* msg_handler = *it;

        if (msg_handler->Initialize(ctx) != 0)
        {
            return -1;
        }

        if (msg_dispatcher_->AttachMsgHandler(msg_handler->GetMsgId(), msg_handler) != 0)
        {
            return -1;
        }
    }

    return 0;
}

template<typename MsgHandler>
void MsgHandlerMgrTemplate<MsgHandler>::Finalize()
{
    for (typename MsgHandlerVec::iterator it = msg_handler_vec_.begin(); it != msg_handler_vec_.end(); ++it)
    {
        MsgHandler* msg_handler = *it;
        msg_dispatcher_->DetachMsgHandler(msg_handler->GetMsgId());
        msg_handler->Finalize();
    }

    FinalizeMsgHandlerVec();
}

template<typename MsgHandler>
int MsgHandlerMgrTemplate<MsgHandler>::Activate()
{
    for (typename MsgHandlerVec::iterator it = msg_handler_vec_.begin(); it != msg_handler_vec_.end(); ++it)
    {
        if ((*it)->Activate() != 0)
        {
            return -1;
        }
    }

    return 0;
}

template<typename MsgHandler>
void MsgHandlerMgrTemplate<MsgHandler>::Freeze()
{
    for (typename MsgHandlerVec::iterator it = msg_handler_vec_.begin(); it != msg_handler_vec_.end(); ++it)
    {
        (*it)->Freeze();
    }
}
}

#endif // THREAD_CENTER_INC_MSG_HANDLER_MGR_TEMPLATE_H_
