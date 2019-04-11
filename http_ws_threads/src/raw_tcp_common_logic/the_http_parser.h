#ifndef HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_HTTP_PARSER_H_
#define HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_HTTP_PARSER_H_

#include <http_parser.h>
#include "conn.h"
#include "http_msg_handler_interface.h"

namespace tcp
{
namespace raw
{
class HTTPWSCommonLogic;
}

namespace http_ws
{
namespace http
{
struct Req
{
    Req();
    ~Req();

    http_method Method;
    unsigned short MajorVersion;
    unsigned short MinorVersion;
    std::string ClientIP;
    std::string Path;
    QueryMap Queries;
    HeaderMap Headers;
    std::string Body;

    void ParseURL(const char* at, size_t length);

    void AddHeader(const std::string& name, const std::string& value)
    {
        this->Headers.insert(HeaderMap::value_type(name, value));
    }

    void ParseClientIP();

    void AppendBody(const char* buffer, size_t count)
    {
        this->Body.append(buffer, count);
    }

    std::string Dump();

private:
    void ParseQuery(const char* at, size_t length);

    std::string schema_;
    std::string host_;
    uint16_t port_;
    std::string query_;
    std::string fragment_;
    std::string user_info_;
};

class Parser
{
public:
    Parser();
    ~Parser();

    void SetRawTCPCommonLogic(tcp::raw::HTTPWSCommonLogic* http_ws_raw_tcp_common_logic)
    {
        http_ws_raw_tcp_common_logic_ = http_ws_raw_tcp_common_logic;
    }

    void SetConnID(ConnID conn_id)
    {
        conn_id_ = conn_id;
    }

    int Execute(const char* buffer, size_t count);

    void SetLastHeaderName(const char* buffer, size_t count)
    {
        last_header_name_.assign(buffer, count);
    }

    const std::string& GetLastHeaderName() const
    {
        return last_header_name_;
    }

    static int OnMessageBegin(struct http_parser* parser); // 解析开始的时候调用
    static int OnURL(struct http_parser* parser, const char* at, size_t length); // 解析url的时候调用
    static int OnHeaderField(struct http_parser* parser, const char* at, size_t length); // 解析http header的field调用
    static int OnHeaderValue(struct http_parser* parser, const char* at, size_t length); // 解析http header的value调用
    static int OnHeadersComplete(struct http_parser* parser); // 解析完成http header调用
    static int OnBody(struct http_parser* parser, const char* at, size_t length); // 解析http body调用
    static int OnMessageComplete(struct http_parser* parser); // 解析完成调用

private:
    tcp::raw::HTTPWSCommonLogic* http_ws_raw_tcp_common_logic_;
    ConnID conn_id_;

    struct http_parser_settings settings_;
    struct http_parser parser_;
    Req http_req_;
    std::string last_header_name_;
    bool complete_;
};
}
}
}

#endif // HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_HTTP_PARSER_H_
