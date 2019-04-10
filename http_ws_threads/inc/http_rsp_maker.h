#ifndef HTTP_WS_THREADS_INC_HTTP_RSP_MAKER_H_
#define HTTP_WS_THREADS_INC_HTTP_RSP_MAKER_H_

#include <http_parser.h>
#include "http_msg_handler_interface.h"

namespace tcp
{
namespace http
{
class RspMaker
{
public:
    RspMaker() : headers_()
    {
        status_ = HTTP_STATUS_OK;
    }

    ~RspMaker() {}

    RspMaker& SetStatus(http_status status)
    {
        status_ = status;
        return *this;
    }

    RspMaker& SetContentType(const std::string& content_type)
    {
        headers_.insert(std::make_pair("Content-Type", content_type));
        return *this;
    }

    RspMaker& SetKeepAlive(bool on)
    {
        if (on)
        {
            headers_.insert(std::make_pair("Connection",  "Keep-Alive"));
        }
        else
        {
            headers_.insert(std::make_pair("Connection", "close"));
        }

        return *this;
    }

    RspMaker& SetNoCache()
    {
        headers_.insert(std::make_pair("Cache-Control", "no-cache"));
        headers_.insert(std::make_pair("Pragma", "no-cache"));
        return *this;
    }

    RspMaker& AddHeader(const std::string& name, const std::string& value)
    {
        headers_.insert(std::make_pair(name, value));
        return *this;
    }

    std::string MakeRsp(const void* body, size_t len)
    {
        char buf[1024] = "";
        int n = snprintf(buf, sizeof(buf), "HTTP/1.1 %d %s\r\n", status_, http_status_str(status_));

        std::string rsp;
        rsp.append(buf, n);

        auto it = headers_.cbegin();
        while (it != headers_.cend())
        {
            n = snprintf(buf, sizeof(buf), "%s: %s\r\n", it->first.c_str(), it->second.c_str());
            rsp.append(buf, n);
            ++it;
        }

        if (nullptr == body || 0 == len)
        {
            return rsp;
        }

        n = snprintf(buf, sizeof(buf), "%lu", len);
        headers_.insert(make_pair("Content-Length", std::string(buf, n)));

        rsp.append("\r\n");
        rsp.append((const char*) body, len);

        return rsp;
    }

private:
    http_status status_;
    Headers headers_;
};
}
}

#endif // HTTP_WS_THREADS_INC_HTTP_RSP_MAKER_H_
