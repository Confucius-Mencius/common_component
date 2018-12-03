#include "udp_conn.h"
#include <unistd.h>
#include <event2/util.h>
#include "log_util.h"
#include "msg_codec_interface.h"

namespace udp
{
Conn::Conn() : client_addr_()
{
    conn_center_ctx_ = NULL;
    sock_fd_ = -1;
    send_buf_ = NULL;
}

Conn::~Conn()
{
}

void Conn::Release()
{
    delete this;
}

int Conn::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    conn_center_ctx_ = (ConnCenterCtx*) ctx;

    LOG_INFO("max udp msg len: " << conn_center_ctx_->max_udp_msg_len);
    send_buf_ = (char*) malloc(conn_center_ctx_->max_udp_msg_len + 1);
    if (NULL == send_buf_)
    {
        const int err = errno;
        LOG_ERROR("failed to create send buf, errno: " << err << ", err msg: " << strerror(errno));
        return -1;
    }

    return 0;
}

void Conn::Finalize()
{
    if (send_buf_ != NULL)
    {
        free(send_buf_);
        send_buf_ = NULL;
    }
}

int Conn::Activate()
{
    return 0;
}

void Conn::Freeze()
{
}

int Conn::Send(const MsgHead& msg_head, const void* msg_body, size_t msg_body_len, int total_retry)
{
    size_t data_len = 0;

    if (conn_center_ctx_->msg_codec->EncodeMsg(&send_buf_, data_len, msg_head, msg_body, msg_body_len) != 0)
    {
        return -1;
    }

    LOG_TRACE("data len: " << data_len);
    send_buf_[data_len] = '\0';

    int ret = -1;
    int err = 0;

    if (-1 == sendto(sock_fd_, send_buf_, data_len, 0, (struct sockaddr*) &client_addr_, sizeof(client_addr_)))
    {
        err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to send msg to " << client_ip_ << ":" << client_port_ << ", by socket, fd: " << sock_fd_
                  << ", errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));

        while (total_retry > 0)
        {
            usleep(1000); // 1000微妙

            if (sendto(sock_fd_, send_buf_, data_len, 0, (struct sockaddr*) &client_addr_, sizeof(client_addr_)) != -1)
            {
                ret = 0;
                break;
            }

            err = EVUTIL_SOCKET_ERROR();
            --total_retry;
            LOG_ERROR("failed to send msg to " << client_ip_ << ":" << client_port_ << ", by socket, fd: " << sock_fd_
                      << ", errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        }
    }
    else
    {
        ret = 0;
    }

    if (0 == ret)
    {
        LOG_TRACE("send msg to " << client_ip_ << ":" << client_port_ << ", socket fd: " << sock_fd_ << ", "
                  << conn_guid_ << ", " << msg_head);
    }

    return ret;
}
}
