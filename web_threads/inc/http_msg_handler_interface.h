#ifndef WEB_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_
#define WEB_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_

#include "web.h"

struct ConnGUID;

namespace tcp
{
namespace web
{
class LogicCtx;

namespace http
{
class MsgHandlerInterface
{
public:
    MsgHandlerInterface()
    {
        logic_ctx_ = nullptr;
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
        if (nullptr == ctx)
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

    virtual const char* GetPath() = 0;
    virtual void OnGet(const ConnGUID* conn_guid, const char* client_ip,
                       const QueryMap& queries, const HeaderMap& headers) {}
    virtual void OnPost(const ConnGUID* conn_guid, const char* client_ip,
                        const QueryMap& queries, const HeaderMap& headers,
                        const char* body, size_t len) {}

protected:
    LogicCtx* logic_ctx_;
};

class MsgDispatcherInterface
{
public:
    virtual ~MsgDispatcherInterface()
    {
    }

    virtual int AttachMsgHandler(const char* path, MsgHandlerInterface* msg_handler) = 0;
    virtual void DetachMsgHandler(const char* path) = 0;
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
}
}

#endif // WEB_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_
