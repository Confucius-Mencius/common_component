#ifndef THREAD_CENTER_INC_MSG_HANDLER_INTERFACE_H_
#define THREAD_CENTER_INC_MSG_HANDLER_INTERFACE_H_

#include "common_define.h"
#include "msg_define.h"

class ThreadInterface;

namespace base
{
class MsgHandlerInterface
{
public:
    MsgHandlerInterface()
    {
    }

    virtual ~MsgHandlerInterface()
    {
    }

    virtual void Release() = 0;
    virtual int Initialize(const void* ctx) = 0;
    virtual void Finalize() = 0;
    virtual int Activate() = 0;
    virtual void Freeze() = 0;

    virtual MsgId GetMsgId() = 0;
    virtual void OnMsg(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                       size_t msg_body_len) = 0;
};

class MsgDispatcherInterface
{
public:
    virtual ~MsgDispatcherInterface()
    {
    }

    virtual int AttachMsgHandler(MsgId msg_id, MsgHandlerInterface* msg_handler) = 0;
    virtual void DetachMsgHandler(MsgId msg_id) = 0;
    virtual int DispatchMsg(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                            size_t msg_body_len) = 0;
};

class MsgHandlerMgrInterface
{
public:
    MsgHandlerMgrInterface()
    {
        msg_dispatcher_ = NULL;
    }

    virtual ~MsgHandlerMgrInterface()
    {
    }

    virtual int Initialize(const void* ctx) = 0;
    virtual void Finalize() = 0;
    virtual int Activate() = 0;
    virtual void Freeze() = 0;

protected:
    MsgDispatcherInterface* msg_dispatcher_;
};
}

#endif // THREAD_CENTER_INC_MSG_HANDLER_INTERFACE_H_
