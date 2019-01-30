#include "buffer_event_conn.h"
#include <string.h>
#include <iomanip>
#include <event2/buffer.h>
#include <event2/event.h>
#include "log_util.h"
#include "thread_sink.h"

#if defined(USE_BUFFEREVENT)
namespace tcp
{
//const static size_t BUFFER_EVENT_MAX_SINGLE_READ = 16384; // 16k
//const static size_t BUFFER_EVENT_MAX_SINGLE_WRITE = 16384;

void BufferEventConn::BufferEventEventCallback(struct bufferevent* buffer_event, short events, void* arg)
{
    const int err = EVUTIL_SOCKET_ERROR();
    const evutil_socket_t sock_fd = bufferevent_getfd(buffer_event);
    ThreadSink* thread_sink = static_cast<BufferEventConn*>(arg)->thread_sink_;

    LOG_TRACE("events occured on socket, fd: " << sock_fd << ", events: "
              << setiosflags(std::ios::showbase) << std::hex << events);

    BaseConn* conn = thread_sink->GetConnMgr()->GetConn(sock_fd);
    if (NULL == conn)
    {
        LOG_ERROR("failed to get tcp conn by socket fd: " << sock_fd);
        return;
    }

    bool closed = false;

    do
    {
        if (events & BEV_EVENT_ERROR)
        {
            if (err != 0)
            {
                LOG_WARN("error occured on socket, fd: " << sock_fd << ", errno: " << err
                         << ", err msg: " << evutil_socket_error_to_string(err));
            }

            closed = true;
            break;
        }

        if (events & BEV_EVENT_EOF)
        {
            LOG_TRACE("tcp conn closed, fd: " << sock_fd);
            closed = true;
            break;
        }

        if (events & BEV_EVENT_READING)
        {
            LOG_TRACE("reading event occurred on socket, fd: " << sock_fd);

            struct evbuffer* input_buf = bufferevent_get_input(buffer_event);
            const size_t input_buf_len = evbuffer_get_length(input_buf);
            LOG_DEBUG("input buf len: " << input_buf_len);

            if (0 == input_buf_len)
            {
                // 这个事件等价于上面的EOF事件,都表示对端关闭了
                LOG_TRACE("tcp conn closed, socket fd: " << sock_fd);
                closed = true;
                break;
            }
        }

        if (events & BEV_EVENT_WRITING)
        {
            LOG_TRACE("writing event occurred on socket, fd:" << sock_fd);
        }

        if (events & BEV_EVENT_TIMEOUT)
        {
            LOG_TRACE("timeout event occurred on socket, fd: " << sock_fd); // TODO 什么时候会出现timeout？逻辑如何处理？
        }
    } while (0);

    if (closed)
    {
        thread_sink->OnClientClosed(conn);
    }
}

void BufferEventConn::BufferEventReadCallback(struct bufferevent* buffer_event, void* arg)
{
    struct evbuffer* input_buf = bufferevent_get_input(buffer_event);
    const evutil_socket_t sock_fd = bufferevent_getfd(buffer_event);
    LOG_TRACE("recv data, socket fd: " << sock_fd << ", input buf len: " << evbuffer_get_length(input_buf));

    ThreadSink* thread_sink = static_cast<BufferEventConn*>(arg)->thread_sink_;

    if (thread_sink->GetThread()->IsStopping())
    {
        LOG_WARN("in stopping status, refuse all client data");
        return;
    }

    ConnMgr* conn_mgr = thread_sink->GetConnMgr();
    BaseConn* conn = conn_mgr->GetConn(sock_fd);
    if (NULL == conn)
    {
        LOG_ERROR("failed to get tcp conn by socket fd: " << sock_fd);
        return;
    }

    if (conn_mgr->UpdateConnStatus(conn->GetConnGUID()->conn_id) != 0)
    {
        thread_sink->OnClientClosed(conn);
        return;
    }

    const size_t input_buf_len = evbuffer_get_length(input_buf);
    LOG_DEBUG("socket fd: " << sock_fd << ", input buf len: " << input_buf_len);

    if (0 == input_buf_len)
    {
        return;
    }

    unsigned char* data_buf = evbuffer_pullup(input_buf, input_buf_len); // 不用手动释放

    // logic处理
    thread_sink->OnRecvClientData(conn->GetConnGUID(), data_buf, input_buf_len);

    if (evbuffer_drain(input_buf, input_buf_len) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to drain data from input buffer, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
    }
}

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
    (void) ctx;

    buffer_event_ = bufferevent_socket_new(
                        thread_sink_->GetThread()->GetThreadEvBase(),
                        sock_fd_,
                        BEV_OPT_CLOSE_ON_FREE);
    if (NULL == buffer_event_)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to create buffer event, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        evutil_closesocket(sock_fd_);
        return -1;
    }

    // libevent源码中已经改为16k
//    LOG_DEBUG("before set, single read size limit: " << bufferevent_get_max_single_read(buffer_event_)
//              << ", single write size limit: " << bufferevent_get_max_single_write(buffer_event_));

//    bufferevent_set_max_single_read(buffer_event_, BUFFER_EVENT_MAX_SINGLE_READ);
//    bufferevent_set_max_single_write(buffer_event_, BUFFER_EVENT_MAX_SINGLE_WRITE);

//    LOG_DEBUG("after set, single read size limit: " << bufferevent_get_max_single_read(buffer_event_)
//              << ", single write size limit: " << bufferevent_get_max_single_write(buffer_event_));

    bufferevent_setcb(buffer_event_, BufferEventConn::BufferEventReadCallback, NULL,
                      BufferEventConn::BufferEventEventCallback, this);

    if (bufferevent_enable(buffer_event_, EV_READ | EV_WRITE | EV_PERSIST) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to enable buffer event reading and writing, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        bufferevent_free(buffer_event_);
        buffer_event_ = NULL;
        return -1;
    }

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
        LOG_TRACE("send ok, " << client_ip_ << ":" << client_port_ << ", socket fd: " << sock_fd_ << ", " << conn_guid_);
        return 0;
    }
}
}
#endif
