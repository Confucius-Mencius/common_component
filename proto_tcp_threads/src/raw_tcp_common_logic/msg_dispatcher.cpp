#include "msg_dispatcher.h"
#include <string.h>
#include <sys/time.h>
#include "conn.h"
#include "log_util.h"

namespace tcp
{
namespace proto
{
MsgDispatcher::MsgDispatcher() : msg_handler_map_()
{
}

MsgDispatcher::~MsgDispatcher()
{
}

int MsgDispatcher::AttachMsgHandler(::proto::MsgID msg_id, ::proto::MsgHandlerInterface* msg_handler)
{
    MsgHandlerMap::iterator it = msg_handler_map_.find(msg_id);
    if (it != msg_handler_map_.end())
    {
        LOG_ERROR("msg handler already exist, msg id: " << msg_id);
        return -1;
    }

    if (!msg_handler_map_.insert(MsgHandlerMap::value_type(msg_id, msg_handler)).second)
    {
        const int err = errno;
        LOG_ERROR("failed to insert to map, msg id: " << msg_id << ", msg handler: " << msg_handler
                  << ", errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    LOG_INFO("attach msg handler ok, msg id: " << msg_id << ", msg handler: " << msg_handler);
    return 0;
}

void MsgDispatcher::DetachMsgHandler(::proto::MsgID msg_id)
{
    MsgHandlerMap::iterator it = msg_handler_map_.find(msg_id);
    if (it != msg_handler_map_.end())
    {
        msg_handler_map_.erase(it);
    }
}

int MsgDispatcher::DispatchMsg(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
{
    MsgHandlerMap::iterator it = msg_handler_map_.find(msg_head.msg_id);
    if (it == msg_handler_map_.end())
    {
        LOG_WARN("failed to get msg handler, msg id: " << msg_head.msg_id);
        return -1;
    }

    struct timeval begin_time;
    gettimeofday(&begin_time, NULL);
    const long begin_millisecond = begin_time.tv_sec * 1000 + begin_time.tv_usec / 1000;

    const ConnGUID* real_conn_guid = NULL;

    if (conn_guid != NULL && conn_guid->conn_id != INVALID_CONN_ID)
    {
        real_conn_guid = conn_guid;
    }

    it->second->OnMsg(real_conn_guid, msg_head, msg_body, msg_body_len);

    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    const long end_millisecond = end_time.tv_sec * 1000 + end_time.tv_usec / 1000;

    LOG_TRACE("msg process time: " << end_millisecond - begin_millisecond << " milliseconds. "
              << conn_guid << ", " << msg_head << ", msg body len: " << msg_body_len);
    return 0;
}
}
}
