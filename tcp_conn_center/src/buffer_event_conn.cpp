#include "buffer_event_conn.h"
#include <string.h>
#include <event2/buffer.h>
#include "log_util.h"

namespace tcp
{
BufferEventConn::BufferEventConn() : client_ip_(""), conn_guid_()
{
    conn_center_ctx_ = NULL;
    sock_fd_ = -1;
    buf_event_ = NULL;
    client_port_ = 0;
    created_time_ = 0;
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
    if (NULL == ctx)
    {
        return -1;
    }

    conn_center_ctx_ = (ConnCenterCtx*) ctx;

    LOG_DEBUG("single read size limit: " << bufferevent_get_max_single_read(buf_event_)
              << ", single write size limit: " << bufferevent_get_max_single_write(buf_event_));

    return 0;
}

void BufferEventConn::Finalize()
{
    if (buf_event_ != NULL)
    {
        bufferevent_free(buf_event_);
        buf_event_ = NULL;
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
    if (evbuffer_add(bufferevent_get_output(buf_event_), data, len) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to send msg to " << client_ip_ << ":" << client_port_ << ", socket fd: " << sock_fd_
                  << ", errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }
    else
    {
        LOG_DEBUG("send msg to " << client_ip_ << ":" << client_port_ << ", socket fd: " << sock_fd_ << ", " << conn_guid_);
        return 0;
    }
}
}
