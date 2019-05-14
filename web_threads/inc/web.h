#ifndef WEB_THREADS_INC_WEB_H_
#define WEB_THREADS_INC_WEB_H_

#include <map>

namespace tcp
{
namespace web
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

#endif // WEB_THREADS_INC_WEB_H_
