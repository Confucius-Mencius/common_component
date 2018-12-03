#include <stdlib.h>
#include <netinet/in.h>
#include "event2/bufferevent.h"
#include "event2/event.h"
#include "event2/buffer.h"

#define MAX_LINE 16384
#define PORT 40713

char Rot13Char(char c)
{
    /* We don't want to use isalpha here; setting the locale would change
     * which characters are considered alphabetical. */
    if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
    {
        return c + 13;
    }
    else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
    {
        return c - 13;
    }
    else
    {
        return c;
    }
}

void ReadCallback(struct bufferevent* bev, void* ctx)
{
    struct evbuffer* input = bufferevent_get_input(bev);
    if (NULL == input)
    {
        return;
    }

    printf("evbuffer_get_length(input): %lu\n", evbuffer_get_length(input));

    struct evbuffer* output = bufferevent_get_output(bev);
    if (NULL == output)
    {
        return;
    }

    char* line;
    size_t n;
    int i;

    while ((line = evbuffer_readln(input, &n, EVBUFFER_EOL_LF))) // evbuffer_readln只是读
    {
        for (i = 0; i < (int) n; ++i)
        {
            line[i] = Rot13Char(line[i]);
        }

        if (evbuffer_add(output, line, n) < 0)
        {
            continue;
        }

        if (evbuffer_add(output, "\n", 1) < 0)
        {
            continue;
        }

        printf("evbuffer_get_length(input): %lu\n", evbuffer_get_length(input));
        free(line);
        printf("evbuffer_get_length(input): %lu\n", evbuffer_get_length(input));
    }

    if (evbuffer_get_length(input) >= MAX_LINE)
    {
        /* Too long; just process what there is and go on so that the buffer
         * doesn't grow infinitely long. */
        char buf[1024];
        while (evbuffer_get_length(input))
        {
            int n = evbuffer_remove(input, buf, sizeof(buf));
            for (i = 0; i < n; ++i)
            {
                buf[i] = Rot13Char(buf[i]);
            }

            printf("evbuffer_get_length(input): %lu\n", evbuffer_get_length(input));
            evbuffer_add(output, buf, n);
        }

        evbuffer_add(output, "\n", 1);
    }
}

void EventCallback(struct bufferevent* bev, short error, void* ctx)
{
    if (error & BEV_EVENT_EOF)
    {
        /* connection has been closed, do any clean up here */
        printf("BEV_EVENT_EOF");
    }
    else if (error & BEV_EVENT_ERROR)
    {
        /* check errno to see what error occurred */
        printf("BEV_EVENT_ERROR");
    }
    else if (error & BEV_EVENT_TIMEOUT)
    {
        /* must be a timeout event handle, handle it */
        printf("BEV_EVENT_TIMEOUT");
    }

    bufferevent_free(bev);
}

static void AcceptCallback(evutil_socket_t listener, short event, void* arg)
{
    struct event_base* ev_base = (struct event_base*) arg;

    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);

    evutil_socket_t fd = accept(listener, (struct sockaddr*) &ss, &slen);
    if (fd < 0)
    {
        perror("accept");
    }
    else
    {
        if (evutil_make_socket_nonblocking(fd) < 0)
        {
            evutil_closesocket(fd);
            return;
        }

        struct bufferevent* buf_event = bufferevent_socket_new(ev_base, fd, BEV_OPT_CLOSE_ON_FREE);
        if (NULL == buf_event)
        {
            evutil_closesocket(fd);
            return;
        }

        bufferevent_setcb(buf_event, ReadCallback, NULL, EventCallback, NULL);
        bufferevent_setwatermark(buf_event, EV_READ, 0, MAX_LINE);

        if (bufferevent_enable(buf_event, EV_READ | EV_WRITE) < 0)
        {
            bufferevent_free(buf_event);
            evutil_closesocket(fd);
            return;
        }
    }
}

void Run(void)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = 0;
    addr.sin_port = htons(PORT);

    evutil_socket_t listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        return;
    }

    if (evutil_make_socket_nonblocking(listen_fd) < 0)
    {
        evutil_closesocket(listen_fd);
        return;
    }

#ifndef WIN32
    {
        int resue_addr = 1;

        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &resue_addr, sizeof(resue_addr)) < 0)
        {
            evutil_closesocket(listen_fd);
            return;
        }
    }
#endif

    if (bind(listen_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0)
    {
        perror("bind");
        evutil_closesocket(listen_fd);
        return;
    }

    if (listen(listen_fd, 16) < 0)
    {
        perror("listen");
        evutil_closesocket(listen_fd);
        return;
    }

    struct event_base* ev_base = event_base_new();
    if (NULL == ev_base)
    {
        evutil_closesocket(listen_fd);
        return;
    }

    struct event* listen_event = event_new(ev_base, listen_fd, EV_READ | EV_PERSIST, AcceptCallback, (void*) ev_base);
    if (NULL == listen_event)
    {
        event_base_free(ev_base);
        evutil_closesocket(listen_fd);
        return;
    }

    if (event_add(listen_event, NULL) < 0)
    {
        event_free(listen_event);
        event_base_free(ev_base);
        evutil_closesocket(listen_fd);
        return;
    }

    event_base_dispatch(ev_base);
}

int main(int c, char** v)
{
    setvbuf(stdout, NULL, _IONBF, 0);
    Run();
    return 0;
}
