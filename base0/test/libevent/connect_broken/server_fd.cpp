#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/bufferevent.h>
#include <unistd.h>
#include "event2/event_struct.h"
#include "event2/buffer.h"
#include <string.h>

#define LISTEN_PORT 9999
#define LISTEN_BACKLOG 32
#define MAX_LINE    256
#define oops(msg) { perror(msg); exit(EXIT_FAILURE); }

static void TimerCallback(evutil_socket_t fd, short event, void* arg)
{
    // 模拟服务器向客户端发心跳。如果客户端拔网线，通过write fd的方式是可以立即报错的，前提是要将fd设为non block的。
    // 附带说明，通过evbuf只会将buf的长度无限增大，而不会感知到网络断开。
    static char* buf = new char[1048576];

    struct bufferevent* bev = (struct bufferevent*) arg;
    evutil_socket_t client_fd = bufferevent_getfd(bev);
    ssize_t n = write(client_fd, buf, 1048576);
    const int err = errno;
    printf("write\n");
    if (n < 0)
    {
        printf("write error: %d, %s\n", err, strerror(err));
        return;
    }

//    struct evbuffer* evbuf = bufferevent_get_output(bev);
//    size_t len = evbuffer_get_length(evbuf);
//    printf("len: %lu\n", len);
}

void ReadCallback(struct bufferevent* bev, void* arg)
{
    struct event_base* ev_base = (struct event_base*) arg;

//    char line[MAX_LINE + 1];
//    int n;
//    evutil_socket_t fd = bufferevent_getfd(bev);
//    ssize_t n = write(fd, "123456", 6);
//    const int err = errno;
//    printf("write\n");
//    if (n < 0)
//    {
//        printf("write error: %d, %s\n", err, strerror(err));
//        return;
//    }

//    n = bufferevent_read(bev, line, MAX_LINE);
//    printf("%d\n", n);
//
//    if (n > 0)
//    {
//        line[n] = '\0';
//        printf("fd=%u, read line: %s\n", fd, line);
//
//        bufferevent_write(bev, line, n);
//    }

    // 测试拔网线这种断开连接的检测机制
    // 添加一个定时器
    struct event* timer_event = event_new(ev_base, -1, EV_PERSIST, TimerCallback, bev);

    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 3;
    tv.tv_usec = 0;

    if (event_add(timer_event, &tv) != 0)
    {
    }

}

void write_cb(struct bufferevent* bev, void* arg)
{
    printf("write callback\n");

//    struct evbuffer* evbuf = bufferevent_get_output(bev);
//    size_t len = evbuffer_get_length(evbuf);
//    printf("len: %lu\n", len);
}

void EventCallback(struct bufferevent* bev, short event, void* arg)
{
    evutil_socket_t fd = bufferevent_getfd(bev);
    printf("fd = %u, ", fd);
    if (event & BEV_EVENT_TIMEOUT)
    {
        printf("Timed out\n"); //if bufferevent_set_timeouts() called
    }
    else if (event & BEV_EVENT_EOF)
    {
        printf("connection closed\n");
    }
    else if (event & BEV_EVENT_ERROR)
    {
        printf("some other error\n");
    }

    bufferevent_free(bev);
}

void AcceptCallback(evutil_socket_t sockfd, short event_type, void* arg)
{
    evutil_socket_t fd;
    struct sockaddr_in sin;
    socklen_t slen;

    fd = accept(sockfd, (struct sockaddr*) &sin, &slen);
    if (fd < 0)
    {
        oops("ERROR: accept: ");
    }

    printf("ACCEPT: fd = %u\n", fd);
    evutil_make_socket_nonblocking(fd);

    struct event_base* ev_base = (struct event_base*) arg;
    struct bufferevent* buf_event = bufferevent_socket_new(ev_base, fd, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_setcb(buf_event, ReadCallback, write_cb, EventCallback, arg);
    bufferevent_enable(buf_event, EV_READ | EV_WRITE | EV_PERSIST);
}

int main(int argc, char** argv)
{
    evutil_socket_t listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        oops("ERROR: socket initiate:");
    }

    evutil_make_socket_nonblocking(listen_fd);
    evutil_make_listen_socket_reuseable(listen_fd);

    struct sockaddr_in address;
    bzero((void*) &address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("0.0.0.0");
    address.sin_port = htons(LISTEN_PORT);

    if (bind(listen_fd, (struct sockaddr*) &address, sizeof(address)) == -1)
    {
        oops("ERROR: bind:");
    }

    if (listen(listen_fd, LISTEN_BACKLOG) == -1)
    {
        oops("ERROR: listen:");
    }

    //================================================
    puts("Listening...");

    struct event_base* ev_base = event_base_new();
    if (ev_base == NULL)
    {
        oops("ERROR: event_base_new() :");
    }

    struct event* listen_event = event_new(ev_base, listen_fd, EV_READ | EV_PERSIST, AcceptCallback, (void*) ev_base);

    event_add(listen_event, NULL); //NULL: no timeout
    event_base_dispatch(ev_base);

    puts("The End.");
    return 0;
}
