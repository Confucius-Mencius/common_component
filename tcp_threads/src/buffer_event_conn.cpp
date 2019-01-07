#include "buffer_event_conn.h"
#include <string.h>
#include <event2/buffer.h>
#include "log_util.h"

namespace tcp
{
BufferEventConn::BufferEventConn()
{
    buffer_event_ = NULL;
}

BufferEventConn::~BufferEventConn()
{
}

void BufferEventConn::Release()
{
    delete this;
}

int BufferEventConn::Initialize(const void* ctx)
{
    LOG_DEBUG("single read size limit: " << bufferevent_get_max_single_read(buffer_event_)
              << ", single write size limit: " << bufferevent_get_max_single_write(buffer_event_));
    return 0;
}

void BufferEventConn::Finalize()
{
    if (buffer_event_ != NULL)
    {
        bufferevent_free(buffer_event_);
        buffer_event_ = NULL;
    }
}

int BufferEventConn::Activate()
{
    return 0;
}

void BufferEventConn::Freeze()
{
}

int BufferEventConn::Send(const void* data, size_t len)
{
    if (evbuffer_add(bufferevent_get_output(buffer_event_), data, len) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to send to " << client_ip_ << ":" << client_port_ << ", socket fd: " << sock_fd_
                  << ", errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }
    else
    {
        LOG_DEBUG("send ok, " << client_ip_ << ":" << client_port_ << ", socket fd: " << sock_fd_ << ", " << conn_guid_);
        return 0;
    }
}
}
