#include "normal_conn.h"
#include <string.h>
#include <unistd.h>
#include <iomanip>
#include "log_util.h"
#include "task_type.h"
#include "thread_sink.h"

namespace tcp
{
void NormalConn::ReadCallback(evutil_socket_t sock_fd, short events, void* arg)
{
    LOG_TRACE("events occured on socket, fd: " << sock_fd << ", events: "
              << setiosflags(std::ios::showbase) << std::hex << events);

    ThreadSink* thread_sink = static_cast<NormalConn*>(arg)->thread_sink_;
    ConnCenter* conn_center = thread_sink->GetConnCenter();

    BaseConn* conn = static_cast<BaseConn*>(conn_center->GetConnBySockFD(sock_fd));
    if (nullptr == conn)
    {
        LOG_ERROR("failed to get tcp conn by socket fd: " << sock_fd);
        return;
    }

    bool closed = false;

    do
    {
        if (events & EV_CLOSED)
        {
            LOG_TRACE("tcp conn closed, fd: " << sock_fd);
            closed = true;
            break;
        }

        if (events & EV_READ)
        {
            if (thread_sink->GetThread()->IsStopping())
            {
                LOG_WARN("in stopping status, refuse all client data");
                break;
            }

            if (conn_center->UpdateConnStatus(conn->GetConnGUID()->conn_id, true) != 0)
            {
                closed = true;
                break;
            }

            LOG_TRACE("recv data, socket fd: " << sock_fd);

            char buf[16384] = "";
            const int max_read_len = sizeof(buf);

            while (true)
            {
                if (nullptr == conn_center->GetConnBySockFD(sock_fd))
                {
                    // conn可能在业务逻辑中被关闭了，相同的socket fd被其他线程重复使用了
                    LOG_WARN("no such socket fd in current thread, socket fd: " << sock_fd);
                    return;
                }

                ssize_t n = read(sock_fd, buf, max_read_len);
                if (0 == n)
                {
                    // 对端关闭了。注：空连接关闭时，不会收到close事件，但会收到read事件，read返回0
                    LOG_TRACE("read 0, eof");
                    closed = true;
                    break;
                }
                else if (n < 0)
                {
                    const int err = errno;

                    if (EINTR == err)
                    {
                        // 被信号中断了，重试
                        LOG_WARN("Interrupted system call, socket fd: " << sock_fd);
                        continue;
                    }
                    else if (EAGAIN == err || EWOULDBLOCK == err)
                    {
                        // socket缓冲区中没有数据可读了
                        LOG_WARN("EAGAIN, socket fd: " << sock_fd);
                        break;
                    }
                    else if (ECONNRESET == err || EPIPE == err)
                    {
                        // 对端关闭了
                        LOG_WARN("conn reset by peer");
                        closed = true;
                        break;
                    }
                    else
                    {
                        // 其它错误
                        LOG_ERROR("read error, n: " << n << ", socket fd: " << sock_fd
                                  << ", errno: " << err << ", err msg: " << strerror(err));
                        break;
                    }
                }

                LOG_DEBUG("recv len: " << n);
                thread_sink->OnRecvClientData(conn->GetConnGUID(), buf, n);
            }
        }

        if (events & EV_TIMEOUT)
        {
            LOG_TRACE("timeout event occurred on socket, fd: " << sock_fd); // TODO 什么时候会出现timeout？逻辑如何处理？
        }
    } while (0);

    if (closed)
    {
        thread_sink->OnClientClosed(conn);
    }
}

void NormalConn::WriteCallback(evutil_socket_t sock_fd, short events, void* arg)
{
    LOG_TRACE("events occured on socket, fd: " << sock_fd << ", events: "
              << setiosflags(std::ios::showbase) << std::hex << events);

    // 在read事件中处理close事件
//    if (events & EV_CLOSED)
//    {
//        return;
//    }

    NormalConn* conn = static_cast<NormalConn*>(arg);

    for (SendList::iterator it = conn->send_list_.begin(); it != conn->send_list_.end();)
    {
        size_t want_send_len = (*it).size();
        size_t sent_len = 0;

        while (true)
        {
            // write的时候对端关闭了，会收到一个RST的响应，服务端再往这个socket写数据时，系统会发出一个SIGPIPE信号给进程，通知进程这个连接已经断开
            ssize_t n = write(sock_fd, ((char*) (*it).data()) + sent_len, want_send_len);
            if (0 == n)
            {
                // 一个字节都没写成功，继续写
                LOG_TRACE("nothing write, continue");
                continue;
            }

            if (n < 0)
            {
                // 出错了
                const int err = errno;
                LOG_ERROR("write error, ret: " << n << ", socked fd: " << sock_fd << ", errno: "
                          << err << ", err msg: " << evutil_socket_error_to_string(err));

                if (EINTR == err)
                {
                    // 被信号中断了，重试
                    // The call was interrupted by a signal before any data was written
                    continue;
                }

                if (EAGAIN == err || EWOULDBLOCK == err)
                {
                    // socket缓冲区满了，等下次回调再写
                    (*it).assign(((char*) (*it).data()) + sent_len, want_send_len); // TODO 重叠assign是否安全？
                }
                else if (ECONNRESET == err || EPIPE == err)
                {
                    // 对端关闭了。此时select/epoll指示socket可读，但是read返回-1，errno为ECONNRESET或EPIPE。在可读时处理对端关闭
                }
                else
                {
                    // 其它错误，等下次回调再写
                    (*it).assign(((char*) (*it).data()) + sent_len, want_send_len); // TODO 重叠assign是否安全？
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
                    LOG_TRACE("all send ok");
                    break;
                }

                want_send_len -= n;
            }
        }

        it = conn->send_list_.erase(it);
    }

    if (conn->send_list_.empty())
    {
        LOG_TRACE("send list is empty, del write event");

        if (event_del(conn->write_event_) != 0)
        {
            const int err = EVUTIL_SOCKET_ERROR();
            LOG_ERROR("failed to del write event, errno: " << err << ", err msg: "
                      << evutil_socket_error_to_string(err));
        }
        else
        {
            LOG_TRACE("del write event ok");
        }
    }

    conn->GetConnCenter()->UpdateConnStatus(conn->GetConnGUID()->conn_id, false);
}

NormalConn::NormalConn() : send_list_()
{
    read_event_ = nullptr;
    write_event_ = nullptr;
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
    read_event_ = event_new(thread_sink_->GetThread()->GetThreadEvBase(),
                            sock_fd_,
                            EV_READ | EV_PERSIST | EV_CLOSED,
                            NormalConn::ReadCallback, this);
    if (nullptr == read_event_)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to create read event, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));

        evutil_closesocket(sock_fd_);
        return -1;
    }

    if (event_add(read_event_, nullptr) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to add event, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));

        evutil_closesocket(sock_fd_);

        event_free(read_event_);
        read_event_ = nullptr;

        return -1;
    }

    return 0;
}

void NormalConn::Finalize()
{
    if (write_event_ != nullptr)
    {
        event_del(write_event_);
        event_free(write_event_);
        write_event_ = nullptr;
    }

    if (read_event_ != nullptr)
    {
        event_del(read_event_);
        event_free(read_event_);
        read_event_ = nullptr;
    }

    evutil_closesocket(sock_fd_);
    sock_fd_ = -1;

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
    if (nullptr == write_event_)
    {
        int ret = -1;

        do
        {
            write_event_ = event_new(event_get_base(read_event_), sock_fd_,
                                     EV_WRITE | EV_PERSIST, // EV_CLOSED 在read事件中处理
                                     NormalConn::WriteCallback, this);
            if (nullptr == write_event_)
            {
                const int err = EVUTIL_SOCKET_ERROR();
                LOG_ERROR("failed to create write event, errno: " << err << ", err msg: "
                          << evutil_socket_error_to_string(err));
                break;
            }

            if (event_add(write_event_, nullptr) != 0)
            {
                const int err = EVUTIL_SOCKET_ERROR();
                LOG_ERROR("failed to add write event, errno: " << err << ", err msg: "
                          << evutil_socket_error_to_string(err));
                break;
            }

            LOG_TRACE("add write event ok");
            ret = 0;
        } while (0);

        if (ret != 0)
        {
            if (write_event_ != nullptr)
            {
                event_free(write_event_);
                write_event_ = nullptr;
            }

            return ret;
        }
    }
    else
    {
        if (send_list_.empty())
        {
            if (event_add(write_event_, nullptr) != 0)
            {
                const int err = EVUTIL_SOCKET_ERROR();
                LOG_ERROR("failed to add write event, errno: " << err << ", err msg: "
                          << evutil_socket_error_to_string(err));

                event_free(write_event_);
                write_event_ = nullptr;
                return -1;
            }

            LOG_TRACE("add write event ok");
        }
    }

    send_list_.push_back(std::string((const char*) data, len));
    return 0;
}
}
