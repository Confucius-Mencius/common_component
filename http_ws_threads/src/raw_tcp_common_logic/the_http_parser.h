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

namespace http
{
class HTTPReq
{
public:
    HTTPReq();
    ~HTTPReq();

    http_method method;
    std::string client_ip;
    std::string url;
    std::string path;
    QueryParams query_params;
    Headers headers;
    std::string body;

    void ParseURL();

    void AddHeader(const std::string& name, const std::string& value)
    {
        headers.insert(std::make_pair(name, value));
    }

    void ParseClientIP();

    std::string Dump();

private:
    void ParseQuery();

    unsigned short major_version_;
    unsigned short minor_version_;

    std::string schema_;
    std::string host_;
    uint16_t port_;
    std::string query_;
    std::string fragment_;
    std::string user_info_;
};

class HTTPParser
{
public:
    HTTPParser();
    ~HTTPParser();

    void SetRawTCPCommonLogic(tcp::raw::HTTPWSCommonLogic* http_ws_raw_tcp_common_logic)
    {
        http_ws_raw_tcp_common_logic_ = http_ws_raw_tcp_common_logic;
    }

    void SetConnID(ConnID conn_id)
    {
        conn_id_ = conn_id;
    }

    int Execute(const char* buffer, size_t count);

private:
    static int OnMessageBegin(struct http_parser* parser); // 解析开始的时候调用
    static int OnURL(struct http_parser* parser, const char* at, size_t length); // 解析url的时候调用
    static int OnHeaderField(struct http_parser* parser, const char* at, size_t length); // 解析http header的field调用
    static int OnHeaderValue(struct http_parser* parser, const char* at, size_t length); // 解析http header的value调用
    static int OnHeadersComplete(struct http_parser* parser); // 解析完成http header调用
    static int OnBody(struct http_parser* parser, const char* at, size_t length); // 解析http body调用
    static int OnMessageComplete(struct http_parser* parser); // 解析完成调用

private:
    struct http_parser_settings parser_settings_;
    struct http_parser http_parser_;
    HTTPReq http_req_;
    std::string last_header_name_;
    tcp::raw::HTTPWSCommonLogic* http_ws_raw_tcp_common_logic_;
    ConnID conn_id_;
};
}
}

#endif // HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_HTTP_PARSER_H_
