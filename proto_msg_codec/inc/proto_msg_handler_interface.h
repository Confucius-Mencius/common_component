#ifndef PROTO_MSG_CODEC_INC_PROTO_MSG_HANDLER_INTERFACE_H_
#define PROTO_MSG_CODEC_INC_PROTO_MSG_HANDLER_INTERFACE_H_

#include "proto_msg.h"

struct ConnGUID;

namespace proto
{
class MsgHandlerInterface
{
public:
    MsgHandlerInterface() {}

    virtual ~MsgHandlerInterface() {}

    virtual void Release() = 0;
    virtual int Initialize(const void* ctx) = 0;
    virtual void Finalize() = 0;
    virtual int Activate() = 0;
    virtual void Freeze() = 0;

    virtual MsgID GetMsgID() = 0;
    virtual void OnMsg(const ConnGUID* conn_guid, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len) = 0;
};

class MsgDispatcherInterface
{
public:
    virtual ~MsgDispatcherInterface()
    {
    }

    virtual int AttachMsgHandler(MsgID msg_id, MsgHandlerInterface* msg_handler) = 0;
    virtual void DetachMsgHandler(MsgID msg_id) = 0;
    virtual int DispatchMsg(const ConnGUID* conn_guid, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len) = 0;
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

#endif // PROTO_MSG_CODEC_INC_PROTO_MSG_HANDLER_INTERFACE_H_
