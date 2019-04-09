#ifndef HTTP_WS_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_
#define HTTP_WS_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_

#include <map>
#include <string.h>
#include <string>
#include <vector>
#include <http_parser.h>

struct ConnGUID;

namespace tcp
{
namespace http
{
class LogicCtx;

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
                       const QueryParams& query_params, const Headers& headers) = 0;
    virtual void OnPost(const ConnGUID* conn_guid, const char* client_ip,
                        const QueryParams& query_params, const Headers& headers, const char* body, size_t len) = 0;

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

class HTTPRsp
{
public:
    HTTPRsp()
    {
        status = HTTP_STATUS_OK;
    }

    ~HTTPRsp() {}

    http_status status;
    std::string body;

    void SetContentType(const std::string& content_type)
    {
        headers_.insert(std::make_pair("Content-Type", content_type));
    }

    void SetKeepAlive(bool on)
    {
        if (on)
        {
            headers_.insert(std::make_pair("Connection",  "Keep-Alive"));
        }
        else
        {
            headers_.insert(std::make_pair("Connection", "close"));
        }
    }

    void SetNoCache()
    {
        headers_.insert(std::make_pair("Cache-Control", "no-cache"));
        headers_.insert(std::make_pair("Pragma", "no-cache"));
    }

    void AddHeader(const std::string& name, const std::string& value)
    {
        headers_.insert(std::make_pair(name, value));
    }

    std::string Dump()
    {
        char buf[1024] = "";
        int n = snprintf(buf, sizeof(buf), "HTTP/1.1 %d %s\r\n", status, http_status_str(status));

        std::string str;
        str.append(buf, n);

        n = snprintf(buf, sizeof(buf), "%d", int(body.size()));
        headers_.insert(make_pair("Content-Length", std::string(buf, n)));

        auto it = headers_.cbegin();
        while (it != headers_.cend())
        {
            n = snprintf(buf, sizeof(buf), "%s: %s\r\n", it->first.c_str(), it->second.c_str());
            str.append(buf, n);
            ++it;
        }

        str.append("\r\n");
        str.append(body);

        return str;
    }

    void Clear()
    {
        status = HTTP_STATUS_OK;
        headers_.clear();
        body.clear();
    }

private:
    Headers headers_;
};
}
}

#endif // HTTP_WS_THREADS_INC_HTTP_MSG_HANDLER_INTERFACE_H_
