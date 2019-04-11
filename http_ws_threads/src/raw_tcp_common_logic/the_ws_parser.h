#ifndef HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_WS_PARSER_H_
#define HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_WS_PARSER_H_

// only RFC6455
#include <string>

namespace tcp
{
namespace http_ws
{
namespace http
{
class Req;
}

namespace ws
{
enum ParseResult
{
    ERROR,
    INCOMPLETE,
    TEXT_FRAME,
    BINARY_FRAME,
    CLOSE_FRAME,
    PING_FRAME,
    PONG_FRAME,
};

struct Parser
{
    Parser();
    ~Parser();

    std::string Payloads;

    int CheckUpgrade(const http::Req& http_req);
    std::string MakeHandshake();
    ParseResult ParseFrame(size_t& offset, const char* data, size_t len);

private:
    std::string key_;
    std::string protocol_;
    bool is_text_frame_; // text or binary
};
}
}
}

#endif // HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_WS_PARSER_H_
