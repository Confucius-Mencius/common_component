#include "listen_thread_sink.h"
#include <arpa/inet.h>
#include <unistd.h>
#include "app_frame_conf_mgr_interface.h"

namespace tcp
{
void ListenThreadSink::ErrorCallback(struct evconnlistener* listener, void* arg)
{
    const int err = EVUTIL_SOCKET_ERROR();
    evutil_socket_t sock_fd = evconnlistener_get_fd(listener);

    LOG_ERROR("failed to listen on socket, fd: " << sock_fd << ", errno: " << err
              << ", err msg: " << evutil_socket_error_to_string(err));
}

void ListenThreadSink::OnAccept(struct evconnlistener* listener, evutil_socket_t sock_fd,
                                struct sockaddr* sock_addr, int sock_addr_len, void* arg)
{
    ListenThreadSink* sink = (ListenThreadSink*) arg;

    if (sink->thread_->IsStopping())
    {
        LOG_WARN("in stopping status, refuse new connection");
        evutil_closesocket(sock_fd);
        return;
    }

    const int tcp_connection_count_limit = sink->threads_ctx_->raw
                                           ? sink->threads_ctx_->conf_mgr->GetRawTcpConnCountLimit()
                                           : sink->threads_ctx_->conf_mgr->GetTcpConnCountLimit();
    if (tcp_connection_count_limit > 0 && sink->online_tcp_conn_count_ >= tcp_connection_count_limit)
    {
        LOG_ERROR("refuse new connection, online tcp conn count: " << sink->online_tcp_conn_count_
                  << ", has already reached the limit");
        evutil_closesocket(sock_fd);
        return;
    }

    struct sockaddr_in* client_addr = (struct sockaddr_in*) sock_addr;
//    char peer_ip[INET_ADDRSTRLEN] = "";
    NewConnCtx new_conn_ctx;
    new_conn_ctx.client_sock_fd = sock_fd;

    if (NULL == evutil_inet_ntop(AF_INET, &(client_addr->sin_addr),
                                 new_conn_ctx.client_ip, sizeof(new_conn_ctx.client_ip)))
    {
        LOG_ERROR("failed to get client ip, socket fd: " << sock_fd);
    }
    else
    {
        new_conn_ctx.client_port = ntohs(client_addr->sin_port);
        LOG_INFO("client connected, ip: " << new_conn_ctx.client_ip << ", port: " << new_conn_ctx.client_port
                 << ", socket fd: " << sock_fd);
    }

    sink->OnClientConnected(&new_conn_ctx);
}

ListenThreadSink::ListenThreadSink()
{
    threads_ctx_ = NULL;
    tcp_thread_group_ = NULL;
    listen_sock_fd_ = -1;
    listener_ = NULL;
    last_tcp_thread_idx_ = 0;
    online_tcp_conn_count_ = 0;
    max_online_tcp_conn_count_ = 0;
}

ListenThreadSink::~ListenThreadSink()
{
}

void ListenThreadSink::Release()
{
    delete this;
}

int ListenThreadSink::OnInitialize(ThreadInterface* thread)
{
    if (ThreadSinkInterface::OnInitialize(thread) != 0)
    {
        return -1;
    }

    const std::string& tcp_addr_port = threads_ctx_->raw ? threads_ctx_->conf_mgr->GetRawTcpAddrPort()
                                       : threads_ctx_->conf_mgr->GetTcpAddrPort();
    LOG_INFO("tcp listen addr port: " << tcp_addr_port);

    listen_sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock_fd_ < 0)
    {
        const int err = errno;
        LOG_ERROR("failed to create tcp listen socket, errno: " << err << ", err msg: " << strerror(errno));
        return -1;
    }

    int ret = -1;

    do
    {
        if (evutil_make_socket_nonblocking(listen_sock_fd_) != 0)
        {
            const int err = EVUTIL_SOCKET_ERROR();
            LOG_ERROR("failed to set tcp listen socket non-blocking, errno: " << err
                      << ", err msg: " << evutil_socket_error_to_string(err));
            break;
        }

        if (evutil_make_listen_socket_reuseable(listen_sock_fd_) != 0)
        {
            const int err = EVUTIL_SOCKET_ERROR();
            LOG_ERROR("failed to set tcp listen socket reusable, errno: " << err
                      << ", err msg: " << evutil_socket_error_to_string(err));
            break;
        }

        if (evutil_make_listen_socket_reuseable_port(listen_sock_fd_) != 0)
        {
            const int err = EVUTIL_SOCKET_ERROR();
            LOG_ERROR("failed to set tcp listen socket reusable port, errno: " << err
                      << ", err msg: " << evutil_socket_error_to_string(err));
            break;
        }

        struct sockaddr_in listen_sock_addr;
        int listen_sock_addr_len = sizeof(listen_sock_addr);

        if (evutil_parse_sockaddr_port(tcp_addr_port.c_str(),
                                       (struct sockaddr*) &listen_sock_addr, &listen_sock_addr_len) != 0)
        {
            const int err = EVUTIL_SOCKET_ERROR();
            LOG_ERROR("failed to parse tcp listen socket addr port: " << tcp_addr_port
                      << ", errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
            break;
        }

        if (bind(listen_sock_fd_, (struct sockaddr*) &listen_sock_addr, listen_sock_addr_len) != 0)
        {
            const int err = errno;
            LOG_ERROR("failed to bind tcp listen socket, errno: " << err
                      << ", err msg: " << evutil_socket_error_to_string(err)
                      << "tcp listen addr port: " << tcp_addr_port);
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        close(listen_sock_fd_);
        listen_sock_fd_ = -1;
    }

    return 0;
}

void ListenThreadSink::OnFinalize()
{
    if (listener_ != NULL)
    {
        evconnlistener_free(listener_);
        listener_ = NULL;
    }

    ThreadSinkInterface::OnFinalize();
}

int ListenThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    listener_ = evconnlistener_new(thread_->GetThreadEvBase(), ListenThreadSink::OnAccept, this,
                                   LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_THREADSAFE, -1, listen_sock_fd_);
    if (NULL == listener_)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to create listener, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    evconnlistener_set_error_cb(listener_, ListenThreadSink::ErrorCallback);
    return 0;
}

void ListenThreadSink::OnFreeze()
{
    ThreadSinkInterface::OnFreeze();
}

void ListenThreadSink::OnThreadStartOK()
{
    ThreadSinkInterface::OnThreadStartOK();

    pthread_mutex_lock(threads_ctx_->frame_threads_mutex);
    ++(*threads_ctx_->frame_threads_count);
    pthread_cond_signal(threads_ctx_->frame_threads_cond);
    pthread_mutex_unlock(threads_ctx_->frame_threads_mutex);
}

void ListenThreadSink::OnStop()
{
    ThreadSinkInterface::OnStop();
}

void ListenThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();
}

void ListenThreadSink::OnTask(const Task* task)
{
    ThreadSinkInterface::OnTask(task);

    switch (task->GetCtx()->task_type)
    {
        case TASK_TYPE_TCP_CONN_CLOSED:
        {

            LOG_INFO("conn closed: " << task->GetCtx()->msg_body); // close by client self or server
            --online_tcp_conn_count_;
        }
        break;

        default:
        {
        }
        break;
    }
}

bool ListenThreadSink::CanExit() const
{
    return true;
}

void ListenThreadSink::OnClientConnected(const NewConnCtx* new_conn_ctx)
{
    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_TCP_CONN_CONNECTED;
    task_ctx.source_thread = thread_;
    task_ctx.msg_body = (char*) new_conn_ctx;
    task_ctx.msg_body_len = sizeof(NewConnCtx);

    Task* task = Task::Create(&task_ctx);
    if (NULL == task)
    {
        return;
    }

    const int tcp_thread_count = threads_ctx_->raw ? threads_ctx_->conf_mgr->GetRawTcpThreadCount()
                                 : threads_ctx_->conf_mgr->GetTcpThreadCount();
    const int tcp_thread_idx = last_tcp_thread_idx_;

    last_tcp_thread_idx_ = (last_tcp_thread_idx_ + 1) % tcp_thread_count;

    if (tcp_thread_group_->PushTaskToThread(task, tcp_thread_idx) != 0)
    {
        task->Release();
        return;
    }

    ++online_tcp_conn_count_;

    if (online_tcp_conn_count_ > max_online_tcp_conn_count_)
    {
        max_online_tcp_conn_count_ = online_tcp_conn_count_;
        LOG_INFO("max online tcp conn count: " << max_online_tcp_conn_count_);
    }
}
}
