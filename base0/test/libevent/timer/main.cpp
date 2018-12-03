#include <stdio.h>
#include <string.h>
#include "event2/event_struct.h"
#include <event2/event.h>

struct timeval g_last_time;

int g_event_is_persistent;

static void TimerCallback(evutil_socket_t fd, short event, void* arg)
{
    struct timeval new_time, difference;

    evutil_gettimeofday(&new_time, NULL);
    evutil_timersub(&new_time, &g_last_time, &difference);
    double elapsed = difference.tv_sec + (difference.tv_usec / 1.0e6);

    printf("TimerCallback called at %d: %.3f seconds elapsed.\n", (int) new_time.tv_sec, elapsed);
    g_last_time = new_time;

    if (!g_event_is_persistent)
    {
        struct event* timer_event = (struct event*) arg;

        struct timeval tv;
        evutil_timerclear(&tv);
        tv.tv_sec = 2;

        event_add(timer_event, &tv);
    }
}

int main(int argc, char** argv)
{
    int flags;

    if (argc == 2 && !strcmp(argv[1], "-p"))
    {
        g_event_is_persistent = 1;
        flags = EV_PERSIST;
    }
    else
    {
        g_event_is_persistent = 0;
        flags = 0;
    }

    printf("%s %#x %s %#x\n", event_get_version(), event_get_version_number(), LIBEVENT_VERSION,
           LIBEVENT_VERSION_NUMBER);

    struct event_base* ev_base = event_base_new();
    if (NULL == ev_base)
    {
        return -1;
    }

    struct event timer_event;
    if (event_assign(&timer_event, ev_base, -1, flags, TimerCallback, (void*) &timer_event) != 0)
    {
        event_base_free(ev_base);
        return -1;
    }

    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 0;
    tv.tv_usec = 1; // 1微妙

    if (event_add(&timer_event, &tv) != 0)
    {
        event_base_free(ev_base);
        return -1;
    }

    evutil_gettimeofday(&g_last_time, NULL);
    event_base_dispatch(ev_base);

    return 0;
}

