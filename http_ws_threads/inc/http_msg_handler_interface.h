#ifndef HTTP_WS_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_
#define HTTP_WS_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_

#include <map>
#include <string.h>
#include <string>
#include <vector>

struct ConnGUID;

namespace tcp
{
namespace http_ws
{
class LogicCtx;
}

namespace http
{
struct CaseKeyCmp
{
    bool operator() (const std::string& p1, const std::string& p2) const
    {
        return strcasecmp(p1.c_str(), p2.c_str()) < 0;
    }
};

typedef std::multimap<std::string, std::string, CaseKeyCmp> Headers;
typedef std::multimap<std::string, std::string> QueryParams;

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

        logic_ctx_ = (tcp::http_ws::LogicCtx*) ctx;
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
                       const QueryParams& query_params, const Headers& headers) {}
    virtual void OnPost(const ConnGUID* conn_guid, const char* client_ip,
                        const QueryParams& query_params, const Headers& headers, const char* body, size_t len) {}

protected:
    tcp::http_ws::LogicCtx* logic_ctx_;
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

#endif // HTTP_WS_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_
