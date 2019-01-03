#include "normal_conn.h"
#include <string.h>
#include <unistd.h>
#include <iomanip>
#include "log_util.h"

namespace tcp
{
void NormalConn::WriteCallback(evutil_socket_t fd, short events, void* arg)
{
    LOG_DEBUG("events occured, socket fd: " << fd << ", events: "
              << setiosflags(std::ios::showbase) << std::hex << events);

    NormalConn* conn = static_cast<NormalConn*>(arg);

    for (SendList::iterator it = conn->send_list_.begin(); it != conn->send_list_.end();)
    {
        size_t want_send_len = (*it).size();
        size_t sent_len = 0;

        while (true)
        {
            // write的时候对端关闭了，会收到一个RST的响应，服务端再往这个socket写数据时，系统会发出一个SIGPIPE信号给进程，通知进程这个连接已经断开
            ssize_t n = write(fd, ((char*) (*it).data()) + sent_len, want_send_len);
            if (0 == n)
            {
                // 一个字节都没写成功，继续写
                LOG_DEBUG("nothing write, continue");
                continue;
            }

            if (n < 0)
            {
                // 出错了
                const int err = errno;

                LOG_ERROR("write error, ret: " << n << ", socked fd: " << fd << ", errno: "
                          << err << ", err msg: " << evutil_socket_error_to_string(err));

                if (EINTR == err)
                {
                    // 被中断了，重试
                    // The call was interrupted by a signal before any data was written
                    continue;
                }

                if (EAGAIN == err || EWOULDBLOCK == err)
                {
                    // socket缓冲区满了，等下次回调再写
                    (*it).assign(((char*) (*it).data()) + sent_len, want_send_len); // todo 重叠assign是否安全？
                }
                else if (ECONNRESET == err || EPIPE == err)
                {
                    // 对端关闭了。此时select/epoll指示socket可读，但是read返回-1，errno为ECONNRESET或EPIPE
                }
                else
                {
                    // 其它错误，等下次回调再写
                    (*it).assign(((char*) (*it).data()) + sent_len, want_send_len); // todo 重叠assign是否安全？
                }

                return;
            }
            else
            {
                LOG_DEBUG("sent len: " << n);
                sent_len += n;

                if (sent_len == (*it).size())
                {
                    // 全部发送OK了
                    LOG_DEBUG("all send ok");
                    break;
                }

                want_send_len -= n;
            }
        }

        it = conn->send_list_.erase(it);
    }

    if (conn->send_list_.empty())
    {
        LOG_DEBUG("send list is empty, del write event");

        if (event_del(conn->write_event_) != 0)
        {
            const int err = EVUTIL_SOCKET_ERROR();
            LOG_ERROR("failed to del write event, errno: " << err << ", err msg: "
                      << evutil_socket_error_to_string(err));
        }
        else
        {
            LOG_DEBUG("del write event ok");
        }
    }
}

NormalConn::NormalConn() : send_list_(), client_ip_(""), conn_guid_()
{
    created_time_ = 0;
    sock_fd_ = -1;
    read_event_ = NULL;
    write_event_ = NULL;
    client_port_ = 0;
}

NormalConn::~NormalConn()
{
}

void NormalConn::Release()
{
    delete this;
}

int NormalConn::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    return 0;
}

void NormalConn::Finalize()
{
    if (write_event_ != NULL)
    {
        event_del(write_event_);
        event_free(write_event_);
        write_event_ = NULL;
    }

    if (read_event_ != NULL)
    {
        event_del(read_event_);
        event_free(read_event_);
        read_event_ = NULL;
    }

    send_list_.clear();
}

int NormalConn::Activate()
{
    return 0;
}

void NormalConn::Freeze()
{
}

int NormalConn::Send(const void* data, size_t len)
{
    if (NULL == write_event_)
    {
        write_event_ = event_new(event_get_base(read_event_), sock_fd_, EV_WRITE | EV_PERSIST,
                                 NormalConn::WriteCallback, this);
        if (NULL == write_event_)
        {
            const int err = EVUTIL_SOCKET_ERROR();
            LOG_ERROR("failed to create write event, errno: " << err << ", err msg: "
                      << evutil_socket_error_to_string(err));
            return -1;
        }

        if (event_add(write_event_, NULL) != 0)
        {
            const int err = EVUTIL_SOCKET_ERROR();
            LOG_ERROR("failed to add write event, errno: " << err << ", err msg: "
                      << evutil_socket_error_to_string(err));

            event_free(write_event_);
            write_event_ = NULL;
            return -1;
        }

        LOG_DEBUG("add write event ok");
    }
    else
    {
        if (send_list_.empty())
        {
            if (event_add(write_event_, NULL) != 0)
            {
                const int err = EVUTIL_SOCKET_ERROR();
                LOG_ERROR("failed to add write event, errno: " << err << ", err msg: "
                          << evutil_socket_error_to_string(err));

                event_free(write_event_);
                write_event_ = NULL;
                return -1;
            }

            LOG_DEBUG("add write event ok");
        }
    }

    send_list_.push_back(std::string((const char*) data, len));
    return 0;
}
}
