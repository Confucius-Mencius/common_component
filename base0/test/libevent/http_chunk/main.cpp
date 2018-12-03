/*
This shows how to use HTTP chunked transfer in libevent. It starts an HTTP server on port 8080; when the client connects, it outputs an message every second. After all messages are outputted, the server closes the HTTP connection.

It has an unsolved crashing problem though... to see it crash, run the next commands in a shell.

See my ML post "after the http client abruptly closes socket libevent (2.0.3-alpha) http server crashes" at http://archives.seul.org/libevent/users/Jan-2010/msg00054.html

Compile with:

        gcc -Wall -g -std=c99 -D_GNU_SOURCE -Iinclude -Llib -levent -o chunk chunk.c

NB: This assumes you have installed libevent 2.0.3-alpha into the current working directory.


Run it through valgrind (you can just use gdb or run it directly):

        LD_LIBRARY_PATH=lib valgrind -v --leak-check=full ./chunk


In a second shell, access it:

        printf 'GET / HTTP/1.0\n\n' | nc localhost 8080

Press Ctrl+C after a couple of seconds.

The HTTP server will crash and burn inside the http_chunked_trickle_cb function:


Program received signal SIGSEGV, Segmentation fault.
0x005fa0e3 in evhttp_send_reply_chunk (req=0x804b930, databuf=0x804bab0) at http.c:2082
2082		struct evbuffer *output = bufferevent_get_output(req->evcon->bufev);
(gdb) bt
#0  0x005fa0e3 in evhttp_send_reply_chunk (req=0x804b930, databuf=0x804bab0) at http.c:2082
#1  0x08048b18 in http_chunked_trickle_cb (fd=-1, events=1, arg=0x804bb68) at chunk.c:47
#2  0x005e8f58 in event_process_active_single_queue (base=0x804b020, flags=<value optimized out>) at event.c:925
#3  event_process_active (base=0x804b020, flags=<value optimized out>) at event.c:978
#4  event_base_loop (base=0x804b020, flags=<value optimized out>) at event.c:1156
#5  0x005e9b95 in event_base_dispatch (event_base=0x804b020) at event.c:1005
#6  0x08048dd6 in main () at chunk.c:132
(gdb)

I'm guessing that the evhttp_request req object is freed before http_chunked_trickle_cb is called. At first I though on using evhttp_request_own(req) would do the trick, but it didn't... So, what you think is going on and how to fix it?

Oh, this code still needs pervasive error checking...
*/
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/http.h>

static char const* const CHUNKS[] = {
    "Preparing...\n",
    "T-10\n",
    "T-9\n",
    "T-8\n",
    "T-7\n",
    "T-6\n",
    "T-5\n",
    "T-4\n",
    "T-3\n",
    "T-2\n",
    "T-1\n",
    "Liftoff!\n"
};

struct chunk_req_state
{
    struct event_base* base;
    struct evhttp_request* req;
    struct event* timer;
    int i;
};

static void
schedule_trickle(struct chunk_req_state* state, int ms)
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = ms * 1000;
    // XXX TODO why no base argument in evtimer_add? well, because it was already given in evtimer_new!
    evtimer_add(state->timer, &tv);
}

static void
http_chunked_trickle_cb(evutil_socket_t fd, short events, void* arg)
{
    struct chunk_req_state* state = arg;
    struct evbuffer* evb = evbuffer_new();

    evbuffer_add_printf(evb, "%s", CHUNKS[state->i]);
    evhttp_send_reply_chunk(state->req, evb);
    evbuffer_free(evb);

    if (++state->i < sizeof(CHUNKS) / sizeof(CHUNKS[0]))
    {
        schedule_trickle(state, 1000);
    }
    else
    {
        evhttp_send_reply_end(state->req);
        // XXX TODO why no evtimer_free?
        event_free(state->timer);
        free(state);
    }
}

static void
http_chunked_cb(struct evhttp_request* req, void* arg)
{
    struct chunk_req_state* state = malloc(sizeof(struct chunk_req_state));

    memset(state, 0, sizeof(struct chunk_req_state));
    state->req = req;
    state->base = arg;
    state->timer = evtimer_new(state->base, http_chunked_trickle_cb, state);

    evhttp_send_reply_start(req, HTTP_OK, "OK");

    schedule_trickle(state, 0);
}

static void
http_quit_cb(struct evhttp_request* req, void* arg)
{
    struct event_base* base = arg;

    struct evbuffer* body = evbuffer_new();
    evbuffer_add_printf(body, "Bye bye\n");
    evhttp_send_reply(req, HTTP_OK, "OK", body);
    evbuffer_free(body);

    struct timeval delay = {2, 0};
    //event_base_loopexit(req->evcon->base, &delay);
    event_base_loopexit(base, &delay);
}

static struct evhttp*
http_setup(const char* address, short port, struct event_base* base)
{
    struct evhttp* http = evhttp_new(base);

    evhttp_bind_socket(http, address, port);

    evhttp_set_cb(http, "/", http_chunked_cb, base);
    evhttp_set_cb(http, "/quit", http_quit_cb, base);

    return http;
}

// TODO XXX if you press Ctrl+C multiple times, this is called, and at the end,
//      valgrind will repost leaks...
static void
sigint_cb(evutil_socket_t sig, short events, void* ptr)
{
    struct event_base* base = ptr;
    struct timeval delay = {2, 0};

    printf("Caught an interrupt signal; exiting cleanly in two seconds.\n");

    event_base_loopexit(base, &delay);
}

static void
ignore_sigpipe(void)
{
    // ignore SIGPIPE (or else it will bring our program down if the client
    // closes its socket).
    // NB: if running under gdb, you might need to issue this gdb command:
    //          handle SIGPIPE nostop noprint pass
    //     because, by default, gdb will stop our program execution (which we
    //     might not want).
    struct sigaction sa;

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_IGN;

    if (sigemptyset(&sa.sa_mask) < 0 || sigaction(SIGPIPE, &sa, 0) < 0)
    {
        perror("Could not ignore the SIGPIPE signal");
        exit(EXIT_FAILURE);
    }
}

int
main(void)
{
    ignore_sigpipe();

    struct event_base* base = event_base_new();

    struct event* sigint_event = evsignal_new(base, SIGINT, sigint_cb, base);
    if (!sigint_event || event_add(sigint_event, NULL) < 0)
    {
        fprintf(stderr, "Could not create or add the SIGINT signal event.\n");
        return -1;
    }

    struct evhttp* http = http_setup("127.0.0.1", 8080, base);

    event_base_dispatch(base);

    event_free(sigint_event);
    evhttp_free(http);
    event_base_free(base);

    return 0;
}
