#ifndef BURDEN_THREADS_INC_BURDEN_MSG_HANDLER_INTERFACE_H_
#define BURDEN_THREADS_INC_BURDEN_MSG_HANDLER_INTERFACE_H_

#include "msg_handler_interface.h"

namespace burden
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

#endif // BURDEN_THREADS_INC_BURDEN_MSG_HANDLER_INTERFACE_H_
