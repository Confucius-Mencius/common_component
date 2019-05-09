#include "http_msg_dispatcher.h"
#include <string.h>
#include <sys/time.h>
#include "log_util.h"

namespace tcp
{
namespace http_ws
{
namespace http
{
MsgDispatcher::MsgDispatcher() : msg_handler_map_()
{
}

MsgDispatcher::~MsgDispatcher()
{
}

int MsgDispatcher::AttachMsgHandler(const char* path, MsgHandlerInterface* msg_handler)
{
    if (nullptr == path || strlen(path) < 1)
    {
        return -1;
    }

    MsgHandlerMap::iterator it = msg_handler_map_.find(path);
    if (it != msg_handler_map_.end())
    {
        LOG_ERROR("msg handler already exist, path: " << path);
        return -1;
    }

    if (!msg_handler_map_.insert(MsgHandlerMap::value_type(path, msg_handler)).second)
    {
        const int err = errno;
        LOG_ERROR("failed to insert to map, path: " << path << ", msg handler: " << msg_handler
                  << ", errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    LOG_ALWAYS("attach msg handler ok, path: " << path << ", msg handler: " << msg_handler);
    return 0;
}

void MsgDispatcher::DetachMsgHandler(const char* path)
{
    if (nullptr == path)
    {
        return;
    }

    MsgHandlerMap::iterator it = msg_handler_map_.find(path);
    if (it != msg_handler_map_.end())
    {
        msg_handler_map_.erase(it);
    }
}

int MsgDispatcher::DispatchMsg(const ConnInterface* conn, const Req& http_req)
{
    MsgHandlerMap::iterator it = msg_handler_map_.find(http_req.Path);
    if (it == msg_handler_map_.end())
    {
        LOG_WARN("failed to get msg handler, path: " << http_req.Path);
        return -1;
    }

    struct timeval begin_time;
    gettimeofday(&begin_time, nullptr);
    const long begin_millisecond = begin_time.tv_sec * 1000 + begin_time.tv_usec / 1000;

    const ConnGUID* conn_guid = conn->GetConnGUID();

    if (HTTP_GET == http_req.Method)
    {
        it->second->OnGet(conn_guid, http_req.ClientIP.empty() ? conn->GetClientIP() : http_req.ClientIP.c_str(),
                          http_req.Queries, http_req.Headers);
    }
    else if (HTTP_POST == http_req.Method)
    {
        it->second->OnPost(conn_guid, http_req.ClientIP.empty() ? conn->GetClientIP() : http_req.ClientIP.c_str(),
                           http_req.Queries, http_req.Headers,
                           http_req.Body.data(), http_req.Body.size());
    }

    struct timeval end_time;
    gettimeofday(&end_time, nullptr);
    const long end_millisecond = end_time.tv_sec * 1000 + end_time.tv_usec / 1000;

    LOG_INFO("msg process time: " << end_millisecond - begin_millisecond << " milliseconds. "
             << ", path: " << it->first);
    return 0;
}
}
}
}
