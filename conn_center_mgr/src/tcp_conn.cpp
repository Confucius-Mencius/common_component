#include "tcp_conn.h"
#include <unistd.h>
#include <iomanip>
#include <event2/buffer.h>
#include "log_util.h"

namespace tcp
{
void Conn::WriteCallback(evutil_socket_t fd, short events, void* arg)
{
    LOG_TRACE("events occured on socket, fd: " << fd << ", events: "
              << setiosflags(std::ios::showbase) << std::hex << events);
    Conn* conn = (Conn*) arg;

    for (SendList::iterator it = conn->send_list_.begin(); it != conn->send_list_.end();)
    {
        size_t want_send_len = (*it).size();
        size_t sent_len = 0;

        while (true)
        {
            ssize_t n = write(fd, ((char*) (*it).data()) + sent_len, want_send_len);
            if (0 == n)
            {
                // 一个字节都没写成功，继续写
                LOG_TRACE("nothing write, continue");
                continue;
            }
            else if (n < 0)
            {
                const int err = errno;
                if (EINTR == err)
                {
                    // 被中断了，可以继续写
                    continue;
                }
                else if (EAGAIN == err)
                {
                    // socket缓冲区满了，等下再写
                    (*it).assign(((char*) (*it).data()) + sent_len, want_send_len);
                    return;
                }
                else
                {
                    // 出错了
                    LOG_ERROR("write error, n: " << n << ", socked fd: " << fd << ", errno: " << err << ", err msg: "
                              << evutil_socket_error_to_string(err));
                    return;
                }
            }
            else
            {
                LOG_TRACE("sent len: " << n);
                sent_len += n;

                if (sent_len == (*it).size())
                {
                    // 全部发送OK了
                    break;
                }

                want_send_len -= sent_len;
            }
        }

        it = conn->send_list_.erase(it);
    }

    if (conn->send_list_.empty())
    {
        LOG_TRACE("send list is empty, del write event");
        event_del(conn->write_event_);
    }
}

Conn::Conn() : conn_center_ctx_(), send_list_()
{
    sock_fd_ = -1;
    buf_event_ = NULL;
    send_buf_ = NULL;
    read_event_ = NULL;
    write_event_ = NULL;
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
    const size_t max_msg_body_len = conn_center_ctx_->max_msg_body_len;

    if (max_msg_body_len > 0)
    {
        LOG_TRACE("tcp max msg body len: " << max_msg_body_len); // raw tcp时该值为0

        send_buf_ = (char*) malloc(MIN_DATA_LEN + max_msg_body_len + 1);
        if (NULL == send_buf_)
        {
            const int err = errno;
            LOG_ERROR("failed to create send buf, errno: " << err << ", err msg: " << strerror(errno));
            return -1;
        }
    }

    return 0;
}

void Conn::Finalize()
{
#if defined(USE_BUFFEREVENT)
    if (buf_event_ != NULL)
    {
        bufferevent_free(buf_event_);
        buf_event_ = NULL;
    }
#else
    if (read_event_ != NULL)
    {
        event_del(read_event_);
        event_free(read_event_);
        read_event_ = NULL;
    }

    if (write_event_ != NULL)
    {
        event_del(write_event_);
        event_free(write_event_);
        write_event_ = NULL;
    }

    send_list_.clear();
#endif

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

    return Send(send_buf_, data_len, total_retry);
}

int Conn::Send(const void* msg, size_t msg_len, int total_retry)
{
#if defined(USE_BUFFEREVENT)
    int ret = -1;
    int err = 0;

    if (evbuffer_add(bufferevent_get_output(buf_event_), msg, msg_len) != 0)
    {
        err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to send msg to " << client_ip_ << ":" << client_port_ << ", by socket, fd: " << sock_fd_
                  << ", errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));

        while (total_retry > 0)
        {
            usleep(1000); // 1000微妙

            if (0 == evbuffer_add(bufferevent_get_output(buf_event_), send_buf_, msg_len))
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
                  << conn_guid_);
    }

    return ret;
#else

    if (NULL == write_event_)
    {
        write_event_ = event_new(event_get_base(read_event_), sock_fd_, EV_WRITE | EV_PERSIST,
                                 Conn::WriteCallback, this);
        if (NULL == write_event_)
        {
            const int err = EVUTIL_SOCKET_ERROR();
            LOG_ERROR("failed to create write event, errno: " << err
                      << ", err msg: " << evutil_socket_error_to_string(err));
            return -1;
        }

        if (event_add(write_event_, NULL) != 0)
        {
            const int err = EVUTIL_SOCKET_ERROR();
            LOG_ERROR("failed to add event, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
            event_free(write_event_);
            return -1;
        }

        LOG_TRACE("add write event ok");
    }
    else
    {
        if (send_list_.empty())
        {
            if (event_add(write_event_, NULL) != 0)
            {
                const int err = EVUTIL_SOCKET_ERROR();
                LOG_ERROR("failed to add event, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
                event_free(write_event_);
                return -1;
            }

            LOG_TRACE("add write event ok");
        }
    }

    send_list_.push_back(std::string((const char*) msg, msg_len));

    return 0;
#endif
}
}
