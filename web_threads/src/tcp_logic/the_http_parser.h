#ifndef WEB_THREADS_SRC_TCP_LOGIC_THE_HTTP_PARSER_H_
#define WEB_THREADS_SRC_TCP_LOGIC_THE_HTTP_PARSER_H_

#include <http_parser.h>
#include "conn.h"
#include "mpart_body_processor.h"
#include "web.h"

namespace tcp
{
class WebLogic;

namespace web
{
namespace http
{
typedef void (*FreeBodyParser) (void*);

struct MPartBodyCtx
{
    MPartBodyProcessor processor;

    int parsed; // parsed / content_length就是进度
    int content_length;
    int socket_fd;

    MPartBodyCtx()
    {
        parsed = 0;
        content_length = 0;
        socket_fd = -1;
    }

    ~MPartBodyCtx() {}

    void Reset()
    {
        parsed = 0;
        content_length = 0;
        socket_fd = -1;
    }
};

struct Req
{
    http_method method; // 1:get 3:post
    unsigned short major_version;
    unsigned short minor_version;
    std::string client_ip;
    std::string url;
    std::string schema;
    std::string host;
    uint16_t port;
    std::string path;
    std::string query;
    QueryMap queries;
    std::string fragment;
    std::string user_info;
    HeaderMap headers;
    bool url_decode;
    std::string body;
    struct MPartBodyCtx mpart_body_ctx;

    Req();
    ~Req();

    void Reset();

    void ParseURL(const char* at, size_t length);
    void ParseQuery();

    void AddHeader(const std::string& name, const std::string& value)
    {
        this->headers.insert(HeaderMap::value_type(name, value));
    }

    void ParseClientIP();

    void AppendBody(const char* buffer, size_t count)
    {
        this->body.append(buffer, count);
    }

    std::string Dump();
};

class Parser
{
public:
    Parser();
    ~Parser();

    void SetWebLogic(tcp::WebLogic* web_logic)
    {
        web_logic_ = web_logic;
    }

    void SetConnID(ConnID conn_id)
    {
        conn_id_ = conn_id;
    }

    int Execute(const char* buffer, size_t count);

    static int OnMessageBegin(struct http_parser* parser); // 解析开始的时候调用
    static int OnURL(struct http_parser* parser, const char* at, size_t length); // 解析url的时候调用
    static int OnHeaderField(struct http_parser* parser, const char* at, size_t length); // 解析http header的field调用
    static int OnHeaderValue(struct http_parser* parser, const char* at, size_t length); // 解析http header的value调用
    static int OnHeadersComplete(struct http_parser* parser); // 解析完成http header调用
    static int OnBody(struct http_parser* parser, const char* at, size_t length); // 解析http body调用
    static int OnMessageComplete(struct http_parser* parser); // 解析整个http请求完成调用
    static int OnMPartBody(struct http_parser* parser, const char* at, size_t length);

private:
    tcp::WebLogic* web_logic_;
    ConnID conn_id_;

    struct http_parser parser_;
    Req http_req_;
    std::string last_header_name_;
    bool complete_;
};
}
}
}

#endif // WEB_THREADS_SRC_TCP_LOGIC_THE_HTTP_PARSER_H_
