#ifndef HTTP_WS_THREADS_INC_HTTP_H_
#define HTTP_WS_THREADS_INC_HTTP_H_

#include <map>

namespace tcp
{
namespace http_ws
{
namespace http
{
typedef std::map<std::string, std::string> HeaderMap;
typedef std::map<std::string, std::string> QueryMap;
}

namespace ws
{
    enum FrameType
    {
        TEXT_FRAME,
        BINARY_FRAME,
    };
}
}
}

#endif // HTTP_WS_THREADS_INC_HTTP_H_
