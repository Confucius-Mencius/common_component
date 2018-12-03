#ifndef BASE_TEST_LIBEVENT_HTTP_SERVER_BUG_HTTP_SERVER_H_
#define BASE_TEST_LIBEVENT_HTTP_SERVER_BUG_HTTP_SERVER_H_

#include "evhttp.h"

class HttpServer
{
public:
    static void* ThreadProc(void* arg);

public:
    static void HttpConnClosedCallback(struct evhttp_connection* conn, void* arg);
    static void HttpGenericCallback(struct evhttp_request* req, void* arg);

public:
    HttpServer();
    ~HttpServer();

    int Initialize(int listen_socket_fd);
    void Finalize();
    virtual int Activate();
    virtual void Freeze();

    void OnHttpConnClosed(struct evhttp_connection* conn);
    void OnHttpReq(struct evhttp_request* req);

private:
    int listen_socket_fd_;

    pthread_t thread_id_;
    struct event_base* thread_event_base_;
    struct evhttp* http_;
};

#endif // BASE_TEST_LIBEVENT_HTTP_SERVER_BUG_HTTP_SERVER_H_
