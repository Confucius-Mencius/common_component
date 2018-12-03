#include <stdio.h>
#include <signal.h>
#include "event2/event_struct.h"
#include "event2/event.h"

#ifdef _EVENT___func__
#define __func__ _EVENT___func__
#endif

int g_called = 0;

static void SignalCallback(evutil_socket_t fd, short event, void* arg)
{
    struct event* signal_event = (struct event*) arg;

    printf("%s: got signal %d\n", __func__, event_get_signal(signal_event));

    if (g_called >= 2)
    {
        event_del(signal_event);
    }

    ++g_called;
}

int main(int argc, char** argv)
{
    struct event_base* ev_base = event_base_new();
    if (NULL == ev_base)
    {
        return -1;
    }

    struct event signal_event;
    if (event_assign(&signal_event, ev_base, SIGINT, EV_SIGNAL | EV_PERSIST, SignalCallback, &signal_event) !=
        0) // 这个用event_assign，是为了将ev_signal_int传递给callback作为参数
    {
        // strerror(errno)
        event_base_free(ev_base);
        return -1;
    }

    if (event_add(&signal_event, NULL) != 0)
    {
        // strerror(errno)
        event_base_free(ev_base);
        return -1;
    }

    event_base_dispatch(ev_base);
    event_base_free(ev_base);

    return 0;
}
