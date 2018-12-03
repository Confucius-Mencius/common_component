#include "http_server.h"
#include <pthread.h>

void* HttpServer::ThreadProc(void* arg)
{
    HttpServer* http_server = (HttpServer*) arg;
    event_base_dispatch(http_server->thread_event_base_);
    pthread_exit((void*) 0);
}

void HttpServer::HttpConnClosedCallback(struct evhttp_connection* conn, void* arg)
{
//    LOG_TRACE("http closed, conn: " << conn << ", arg: " << arg);

    if (NULL == conn)
    {
//        LOG_TRACE("param is null");
        return;
    }

    HttpServer* http_server = (HttpServer*) arg;
    http_server->OnHttpConnClosed(conn);
}

void HttpServer::HttpGenericCallback(struct evhttp_request* req, void* arg)
{
//    LOG_TRACE("recv http req, req: " << req << ", arg: " << arg);

    if (NULL == req)
    {
//        LOG_TRACE("param is null");
        return;
    }

//    LOG_TRACE("http version, major: " << (int) req->major << ", minor: " << (int) req->minor << ", flags: " << req->flags);
    HttpServer* http_server = (HttpServer*) arg;
    http_server->OnHttpReq(req);
}

HttpServer::HttpServer()
{
    listen_socket_fd_ = -1;
    thread_id_ = 0;
    thread_event_base_ = NULL;
    http_ = NULL;
}

HttpServer::~HttpServer()
{

}

int HttpServer::Initialize(int listen_socket_fd)
{
    listen_socket_fd_ = listen_socket_fd;

    thread_event_base_ = event_base_new();
    if (NULL == thread_event_base_)
    {
//        LOG_ERROR("failed to create thread event base: " << strerror(errno));
        return -1;
    }

    http_ = evhttp_new(thread_event_base_);
    if (NULL == http_)
    {
//        LOG_ERROR("failed to create http");
        return -1;
    }

    if (evhttp_accept_socket(http_, listen_socket_fd_) != 0)
    {
//        const int err = errno;
//        LOG_ERROR("failed to call evhttp_accept_socket, errno: " << err << ", err: " << strerror(err));
        return -1;
    }

    evhttp_set_gencb(http_, HttpServer::HttpGenericCallback, this);
    return 0;
}

void HttpServer::Finalize()
{
    if (http_ != NULL)
    {
        evhttp_free(http_);
        http_ = NULL;
    }

    if (thread_event_base_ != NULL)
    {
        event_base_free(thread_event_base_);
        thread_event_base_ = NULL;
    }
}

int HttpServer::Activate()
{
    if (pthread_create(&thread_id_, NULL, ThreadProc, this) != 0)
    {
//        LOG_ERROR("failed to start io thread: " << strerror(errno));
        return -1;
    }

    return 0;
}

void HttpServer::Freeze()
{
    pthread_join(thread_id_, NULL);
}

void HttpServer::OnHttpConnClosed(struct evhttp_connection* conn)
{
//    LOG_TRACE("conn: " << conn);
}

void HttpServer::OnHttpReq(struct evhttp_request* req)
{
    struct evhttp_connection* conn = evhttp_request_get_connection(req);
    if (NULL == conn)
    {
//        const int err = errno;
//        LOG_ERROR("failed to get conn, errno: " << err << ", err msg: " << strerror(err));
        return;
    }

    evhttp_connection_set_closecb(conn, HttpServer::HttpConnClosedCallback, this);

    for (int i = 0; i < 10000; ++i)
    { ; // fake logic
    }

    evhttp_send_reply(req, HTTP_OK, NULL, NULL);
}
