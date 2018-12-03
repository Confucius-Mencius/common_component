#ifndef HTTP_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_
#define HTTP_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_

#include "common_define.h"

struct evhttp_request;

namespace http
{
class LogicCtx;

class MsgHandlerInterface
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

    virtual const char* GetHttpReqPath() = 0;

    virtual void OnHttpHeadReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                               const KeyValMap* http_header_map, const KeyValMap* http_query_map) = 0;

    virtual void OnHttpGetReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                              const KeyValMap* http_header_map, const KeyValMap* http_query_map) = 0;

    virtual void OnHttpPostReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                               const KeyValMap* http_header_map, const KeyValMap* http_query_map,
                               const char* data, int data_len) = 0;

    virtual void OnHttpPutReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                              const KeyValMap* http_header_map, const KeyValMap* http_query_map,
                              const char* data, int data_len) = 0;

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
    virtual int AttachGeneralMsgHandler(MsgHandlerInterface* general_msg_handler) = 0;
    virtual void DetachGeneralMsgHandler() = 0;
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

#endif // HTTP_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_
