#ifndef HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_WS_PARSER_H_
#define HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_WS_PARSER_H_

// only RFC6455
#include <string>

namespace tcp
{
namespace http
{
class HTTPReq;
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

/*
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
|N|V|V|V|       |S|             |   (if payload len==126/127)   |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
|     Extended payload length continued, if payload len == 127  |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                               |Masking-key, if MASK set to 1  |
+-------------------------------+-------------------------------+
| Masking-key (continued)       |          Payload Data         |
+-------------------------------- - - - - - - - - - - - - - - - +
:                     Payload Data continued ...                :
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
|                     Payload Data continued ...                |
+---------------------------------------------------------------+
*/

class WSParser
{
public:
    WSParser();
    ~WSParser();

    std::string key;
    std::string protocol;
    std::string payloads;
    bool is_text_frame; // text or binary

    int CheckUpgrade(const http::HTTPReq& http_req);
    std::string MakeHandshake();
    ParseResult ParseFrame(size_t& offset, const char* data, size_t len);
};
}
}

#endif // HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_WS_PARSER_H_
