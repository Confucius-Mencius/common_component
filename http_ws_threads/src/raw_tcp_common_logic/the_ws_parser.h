#ifndef HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_WS_PARSER_H_
#define HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_WS_PARSER_H_

/*
0               1               2               3
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

#include "conn.h"
#include "websocket_parser.h"

// only RFC6455
#include <string>

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
class Req;
}

namespace ws
{
struct Parser
{
    Parser();
    ~Parser();

    int CheckUpgrade(const http::Req& http_req);
    std::string MakeHandshake();

    void SetRawTCPCommonLogic(tcp::raw::HTTPWSCommonLogic* http_ws_raw_tcp_common_logic)
    {
        http_ws_raw_tcp_common_logic_ = http_ws_raw_tcp_common_logic;
    }

    void SetConnID(ConnID conn_id)
    {
        conn_id_ = conn_id;
    }

    int Execute(const char* buffer, size_t count);

    static int OnFrameHeader(websocket_parser* parser);
    static int OnFrameBody(websocket_parser* parser, const char* at, size_t length);
    static int OnFrameEnd(websocket_parser* parser);

private:
    std::string key_;
    std::string protocol_;

    tcp::raw::HTTPWSCommonLogic* http_ws_raw_tcp_common_logic_;
    ConnID conn_id_;

    struct websocket_parser parser_;
    int opcode_;
    int fin_;
    std::string body_;
    std::string payloads_;
};
}
}
}

#endif // HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_THE_WS_PARSER_H_
