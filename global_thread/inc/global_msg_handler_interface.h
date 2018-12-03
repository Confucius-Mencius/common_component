#ifndef GLOBAL_THREAD_INC_GLOBAL_MSG_HANDLER_INTERFACE_H_
#define GLOBAL_THREAD_INC_GLOBAL_MSG_HANDLER_INTERFACE_H_

#include "msg_handler_interface.h"

namespace global
{
class LogicCtx;

class MsgHandlerInterface : public base::MsgHandlerInterface
{
public:
    MsgHandlerInterface()
    {
        logic_ctx_ = NULL;
    }

    virtual ~MsgHandlerInterface()
    {
    }

    ///////////////////////// MsgHandlerInterface /////////////////////////
    virtual void Release()
    {
    }

    virtual int Initialize(const void* ctx)
    {
        if (NULL == ctx)
        {
            return -1;
        }

        logic_ctx_ = (LogicCtx*) ctx;
        return 0;
    }

    virtual void Finalize()
    {
    }

    virtual int Activate()
    {
        return 0;
    }

    virtual void Freeze()
    {
    }

protected:
    LogicCtx* logic_ctx_;
};
}

#endif // GLOBAL_THREAD_INC_GLOBAL_MSG_HANDLER_INTERFACE_H_
