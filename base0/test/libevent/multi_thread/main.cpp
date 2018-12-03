#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif // _GNU_SOURCE

#include <fcntl.h>              /* Obtain O_* constant definitions */
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <event2/event.h>
#include <event2/event_struct.h>
#include <iostream>

// 主线程根据负载均衡算法选择工作线程，每隔一秒钟向特定的工作线程发送一个buf的内容，工作线程简单的把buf的内容打印出来，然后给主线程发一条消息。

const int BUF_SIZE = 64 * 1024;

typedef struct
{
    pthread_t thread_id;
    struct event_base* worker_thread_ev_base; // 每个thread一个event_base
    int read_from_dispatcher_thread_fd;
    int write_to_worker_thread_fd;
    int read_from_worker_thread_fd;
    int write_to_dispatcher_thread_fd;
    struct event read_from_dispatcher_thread_event;
    struct event read_from_worker_thread_event;
} WorkerThread;

typedef struct
{
    pthread_t thread_id;
    struct event_base* dispatcher_thread_ev_base;
} DispatcherThread;

const int WORKER_THREAD_NUM = 10;
WorkerThread* g_worker_threads;
DispatcherThread g_dispatcher_thread;
int g_last_worker_thread = 0;

static void WorkerThreadReadCallback(int fd, short which, void* arg)
{
//    WorkerThread* me = (WorkerThread*) arg;
//    if (fd != me->read_from_dispatcher_thread_fd)
//    {
//        printf("WorkerThreadReadCallback error : fd != me->read_from_dispatcher_thread_fd\n");
//        return;
//    }
//
//    char* buf = new char[BUF_SIZE];
//    if (buf != NULL)
//    {
//        int ret = read(fd, buf, BUF_SIZE);
//        if (ret > 0)
//        {
//            buf[ret] = '\0';
//            printf("thread %llu receive message : %s\n", (unsigned long long) me->thread_id, buf);
//
//            int n = snprintf(buf, BUF_SIZE, "worker thread id: %llu", (unsigned long long) me->thread_id);
//            ssize_t sz = write(me->write_to_dispatcher_thread_fd, buf, n);
//            (void) sz;
//        }
//
//        delete[] buf;
//    }
}

static void DispatcherThreadReadCallback(int fd, short which, void* arg)
{
    char* buf = new char[BUF_SIZE];
    if (buf != NULL)
    {
        int ret = read(fd, buf, BUF_SIZE);
        if (ret > 0)
        {
            buf[ret] = '\0';
            printf("main threa receive message : %s\n", buf);
        }

        delete[] buf;
    }
}

static void* WorkerThreadProc(void* arg)
{
    WorkerThread* me = (WorkerThread*) arg;
    me->thread_id = pthread_self();

    event_base_loop(me->worker_thread_ev_base, 0);

    return (void*) 0;
}

static void TimerCallback(int fd, short event, void* arg)
{
    struct timeval tv;
    struct event* timer_event = (struct event*) arg;

    int t = (g_last_worker_thread + 1) % WORKER_THREAD_NUM;
    WorkerThread* thread = g_worker_threads + t;
    g_last_worker_thread = t;

//    char* buf = new char[BUF_SIZE];
//    if (buf != NULL)
//    {
//        memset(buf, 1, BUF_SIZE);
//        ssize_t sz = write(thread->write_to_worker_thread_fd, buf, BUF_SIZE);
//        sz = write(thread->write_to_worker_thread_fd, buf, BUF_SIZE);
//        (void) sz;
//        delete[] buf;
//    }
    char buf[1];
    buf[0] = 'w';
    if (write(thread->write_to_worker_thread_fd, buf, 1) != 1)
    {
        int e = errno;
        std::cout << "write error: " << e << ", " << strerror(e) << std::endl;
    }
    else
    {
        std::cout << "write ok" << std::endl;
    }

    if (0)
    {
        evutil_timerclear(&tv);
        tv.tv_sec = 1;
        event_add(timer_event, &tv);
    }
}

int main(int argc, char* argv[])
{
    g_dispatcher_thread.dispatcher_thread_ev_base = event_base_new();
    if (g_dispatcher_thread.dispatcher_thread_ev_base == NULL)
    {
        perror("event base");
        return 1;
    }

    g_dispatcher_thread.thread_id = pthread_self();

    struct event timer_event;
    if (event_assign(&timer_event, g_dispatcher_thread.dispatcher_thread_ev_base, -1, EV_PERSIST, TimerCallback,
                     &timer_event) != 0)
    {
        event_base_free(g_dispatcher_thread.dispatcher_thread_ev_base);
        return -1;
    }

    struct timeval tv;
    evutil_timerclear(&tv);
    tv.tv_sec = 0;
    tv.tv_usec = 1;

    if (event_add(&timer_event, &tv) != 0)
    {
        event_base_free(g_dispatcher_thread.dispatcher_thread_ev_base);
        return -1;
    }

    g_worker_threads = (WorkerThread*) calloc(WORKER_THREAD_NUM, sizeof(WorkerThread));
    if (g_worker_threads == NULL)
    {
        perror("calloc");
        event_base_free(g_dispatcher_thread.dispatcher_thread_ev_base);
        return 1;
    }

    for (int i = 0; i < WORKER_THREAD_NUM; ++i)
    {
        int fd[2];

//        int ret = socketpair(AF_LOCAL, SOCK_STREAM, 0, fd);
        int ret = pipe2(fd, O_NONBLOCK); // pipe的能力要优于socketpair
        if (ret == -1)
        {
            perror("socketpair()");
            event_base_free(g_dispatcher_thread.dispatcher_thread_ev_base);
            free(g_worker_threads);
            return 1;
        }

        g_worker_threads[i].read_from_dispatcher_thread_fd = fd[0];
        g_worker_threads[i].write_to_worker_thread_fd = fd[1];

        ret = pipe2(fd, O_NONBLOCK);
        if (ret == -1)
        {
            perror("socketpair()");
            close(g_worker_threads[i].read_from_dispatcher_thread_fd);
            close(g_worker_threads[i].write_to_worker_thread_fd);
            event_base_free(g_dispatcher_thread.dispatcher_thread_ev_base);
            free(g_worker_threads);
            return 1;
        }

        g_worker_threads[i].read_from_worker_thread_fd = fd[0];
        g_worker_threads[i].write_to_dispatcher_thread_fd = fd[1];

        g_worker_threads[i].worker_thread_ev_base = event_base_new();
        if (g_worker_threads[i].worker_thread_ev_base == NULL)
        {
            perror("event_init()");
            close(g_worker_threads[i].read_from_worker_thread_fd);
            close(g_worker_threads[i].write_to_dispatcher_thread_fd);
            close(g_worker_threads[i].read_from_dispatcher_thread_fd);
            close(g_worker_threads[i].write_to_worker_thread_fd);
            event_base_free(g_dispatcher_thread.dispatcher_thread_ev_base);
            free(g_worker_threads);
            return 1;
        }

        //
        event_assign(&g_worker_threads[i].read_from_dispatcher_thread_event,
                     g_worker_threads[i].worker_thread_ev_base,
                     g_worker_threads[i].read_from_dispatcher_thread_fd,
                     EV_READ | EV_PERSIST, WorkerThreadReadCallback,
                     &g_worker_threads[i]);

        if (event_add(&g_worker_threads[i].read_from_dispatcher_thread_event, 0) == -1)
        {
            perror("event_add()");
            close(g_worker_threads[i].read_from_worker_thread_fd);
            close(g_worker_threads[i].write_to_dispatcher_thread_fd);
            close(g_worker_threads[i].read_from_dispatcher_thread_fd);
            close(g_worker_threads[i].write_to_worker_thread_fd);
            event_base_free(g_dispatcher_thread.dispatcher_thread_ev_base);
            free(g_worker_threads);
            return 1;
        }

        //
        event_assign(&g_worker_threads[i].read_from_worker_thread_event,
                     g_dispatcher_thread.dispatcher_thread_ev_base,
                     g_worker_threads[i].read_from_worker_thread_fd,
                     EV_READ | EV_PERSIST, DispatcherThreadReadCallback, NULL);

        if (event_add(&g_worker_threads[i].read_from_worker_thread_event, 0) == -1)
        {
            perror("event_add()");
            close(g_worker_threads[i].read_from_worker_thread_fd);
            close(g_worker_threads[i].write_to_dispatcher_thread_fd);
            close(g_worker_threads[i].read_from_dispatcher_thread_fd);
            close(g_worker_threads[i].write_to_worker_thread_fd);
            event_base_free(g_dispatcher_thread.dispatcher_thread_ev_base);
            free(g_worker_threads);
            return 1;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, WorkerThreadProc, &g_worker_threads[i]);
    }

    event_base_loop(g_dispatcher_thread.dispatcher_thread_ev_base, 0);

    return 0;
}
