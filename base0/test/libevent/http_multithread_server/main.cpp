#include <evhttp.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>

// 本程序测试过，性能很高

int httpserver_bindsocket(int port, int backlog);
int httpserver_start(int port, int nthreads, int backlog);
void* httpserver_Dispatch(void* arg);
void httpserver_GenericHandler(struct evhttp_request* req, void* arg);
void httpserver_ProcessRequest(struct evhttp_request* req);

int httpserver_bindsocket(int port, int backlog)
{
    int r;
    int nfd;
    nfd = socket(AF_INET, SOCK_STREAM, 0);
    if (nfd < 0)
    { return -1; }

    int one = 1;
    r = setsockopt(nfd, SOL_SOCKET, SO_REUSEADDR, (char*) &one, sizeof(int));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    r = bind(nfd, (struct sockaddr*) &addr, sizeof(addr));
    if (r < 0)
    { return -1; }
    r = listen(nfd, backlog);
    if (r < 0)
    { return -1; }

    int flags;
    if ((flags = fcntl(nfd, F_GETFL, 0)) < 0
        || fcntl(nfd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        return -1;
    }

    return nfd;
}

int httpserver_start(int port, int nthreads, int backlog)
{
    int r, i;
    int listen_fd = httpserver_bindsocket(port, backlog);
    if (listen_fd < 0)
    { return -1; }
    pthread_t ths[nthreads];
    for (i = 0; i < nthreads; i++)
    {
        struct event_base* thread_event_base = event_init();
        if (thread_event_base == NULL)
        { return -1; }
        struct evhttp* thread_httpd = evhttp_new(thread_event_base);
        if (thread_httpd == NULL)
        { return -1; }
        r = evhttp_accept_socket(thread_httpd, listen_fd);
        if (r != 0)
        { return -1; }
        evhttp_set_gencb(thread_httpd, httpserver_GenericHandler, NULL);
        r = pthread_create(&ths[i], NULL, httpserver_Dispatch, thread_event_base);
        if (r != 0)
        { return -1; }
    }
    for (i = 0; i < nthreads; i++)
    {
        pthread_join(ths[i], NULL);
    }

    return 0;
}

void* httpserver_Dispatch(void* arg)
{
    event_base_dispatch((struct event_base*) arg);
    return NULL;
}

void httpserver_GenericHandler(struct evhttp_request* req, void* arg)
{
    httpserver_ProcessRequest(req);
}

void httpserver_ProcessRequest(struct evhttp_request* req)
{
    struct evbuffer* buf = evbuffer_new();
    if (buf == NULL)
    { return; }

    const char* uri = evhttp_request_get_uri(req);
    (void) uri;
    const struct evhttp_uri* evuri = evhttp_request_get_evhttp_uri(req);
    (void) evuri;

    //here comes the magic
    evhttp_send_reply(req, HTTP_OK, "OK", NULL);

}

int main(void)
{
    httpserver_start(8009, 1, 10240);
}
