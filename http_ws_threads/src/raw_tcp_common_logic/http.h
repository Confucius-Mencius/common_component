#ifndef HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_HTTP_H_
#define HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_HTTP_H_

namespace tcp
{
namespace http_ws
{
class HTTPParser
{
public:
    HttpParser();

private:
    http_parser_settings parser_settings;
};
}
}

#endif // HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_HTTP_H_
