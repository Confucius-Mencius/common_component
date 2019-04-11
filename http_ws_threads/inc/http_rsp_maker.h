#ifndef HTTP_WS_THREADS_INC_HTTP_RSP_MAKER_H_
#define HTTP_WS_THREADS_INC_HTTP_RSP_MAKER_H_

#include <http_parser.h>
#include "http_msg_handler_interface.h"

namespace tcp
{
namespace http_ws
{
namespace http
{
class RspMaker
{
public:
    RspMaker() : headers_()
    {
        status_code_ = HTTP_STATUS_OK;
    }

    ~RspMaker() {}

    RspMaker& SetStatusCode(http_status status_code)
    {
        status_code_ = status_code;
        return *this;
    }

    RspMaker& SetContentType(const std::string& content_type)
    {
        headers_.insert(HeaderMap::value_type("Content-Type", content_type));
        return *this;
    }

    RspMaker& SetKeepAlive(bool on)
    {
        if (on)
        {
            headers_.insert(HeaderMap::value_type("Connection",  "Keep-Alive"));
        }
        else
        {
            headers_.insert(HeaderMap::value_type("Connection", "close"));
        }

        return *this;
    }

    RspMaker& SetNoCache()
    {
        headers_.insert(HeaderMap::value_type("Cache-Control", "no-cache"));
        headers_.insert(HeaderMap::value_type("Pragma", "no-cache"));
        return *this;
    }

    RspMaker& AddHeader(const std::string& name, const std::string& value)
    {
        headers_.insert(HeaderMap::value_type(name, value));
        return *this;
    }

    std::string MakeRsp(const void* body, size_t len)
    {
        char buf[1024] = "";
        int n = snprintf(buf, sizeof(buf), "HTTP/1.1 %d %s\r\n", status_code_, http_status_str(status_code_));

        std::string rsp(buf, n);

        if (nullptr == body || 0 == len)
        {
            return rsp;
        }

        n = snprintf(buf, sizeof(buf), "%lu", len);
        headers_.insert(HeaderMap::value_type("Content-Length", std::string(buf, n)));

        for (HeaderMap::const_iterator it = headers_.cbegin(); it != headers_.cend(); ++it)
        {
            n = snprintf(buf, sizeof(buf), "%s: %s\r\n", it->first.c_str(), it->second.c_str());
            rsp.append(buf, n);
        }

        rsp.append("\r\n");
        rsp.append((const char*) body, len);

        return rsp;
    }

private:
    http_status status_code_;
    HeaderMap headers_;
};
}
}
}

#endif // HTTP_WS_THREADS_INC_HTTP_RSP_MAKER_H_
