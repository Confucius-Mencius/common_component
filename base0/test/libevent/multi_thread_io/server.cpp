#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include "event2/listener.h"
#include "log4cplus/logger.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/loggingmacros.h"

log4cplus::Logger g_test_logger;

int init_count = 0;
pthread_mutex_t g_io_thread_init_lock;
pthread_cond_t g_io_thread_init_cond;

using namespace std;

struct IOThread
{
    int listen_fd;
    pthread_t io_thread_id;
    struct event_base* io_event_base;
};

void OnClientDataComing(struct bufferevent* buf_event, void* arg)
{
    IOThread* io_thread = (IOThread*) arg;
    LOG4CPLUS_INFO(g_test_logger, "client data comming, I am in the thread: " << io_thread->io_thread_id);

//    CIOEventSink* ioEventSink = (CIOEventSink*) ctx;
//
//    TClientSocketCtx clientSocketCtx;
//    clientSocketCtx.Reset();
//
//    clientSocketCtx._socketFD = bufferevent_getfd(buf_event);
//    clientSocketCtx._bufEvent = buf_event;
//
    struct evbuffer* ev_buf = bufferevent_get_input(buf_event);

//    while (true)
//    {
//        if (!__IsWholeMsg(msgLen, ev_buf))
//        {
//            return;
//        }

    char buf[1024] = "";

    evbuffer_remove(ev_buf, buf, 1024);

    LOG4CPLUS_INFO(g_test_logger, "recv data from client: " << buf);

    std::string str = "I am server";
    evbuffer_add(bufferevent_get_output(buf_event), str.data(), str.size());

//        {
////            LOG_ERROR("Failed to remove data from evbuffer!");
//            return;
//        }

//        msgBuf._dataLen = msgLen;
//        ioEventSink->OnRecvClientMsg(&clientSocketCtx, sizeof(clientSocketCtx),
//                                     msgBuf._data, msgBuf._dataLen);
//
//        OnGameSrvDataComing(0, 0, ioEventSink);
//    }
}

void OnClientError(struct bufferevent* buf_event, short events, void* arg)
{
//    IOThread* io_thread = (IOThread*) arg;

//    CIOEventSink* ioEventSink = (CIOEventSink*) ctx;
//
    const evutil_socket_t fd = bufferevent_getfd(buf_event);
//
//    TClientSocketCtx clientSocketCtx;
//    clientSocketCtx.Reset();
//
//    clientSocketCtx._socketFD = fd;
//    clientSocketCtx._bufEvent = buf_event;
//

    if (events & BEV_EVENT_ERROR)
    {
        LOG4CPLUS_ERROR(g_test_logger, "error occured on client socket fd [" << fd << "]: "
            << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
    }

    if (events & (BEV_EVENT_EOF))
    {
        LOG4CPLUS_INFO(g_test_logger, "xxxxxxxxxxxxxclient closed ");
    }

    if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR))
    {
        // 应该两个都判断，都表示连接失效了
    }
}

void OnAccept(struct evconnlistener* listener, evutil_socket_t fd, struct sockaddr* sock_addr, int sock_addr_len,
              void* arg)
{
    IOThread* io_thread = (IOThread*) arg;
    LOG4CPLUS_INFO(g_test_logger, "on accept, I am in the thread: " << io_thread->io_thread_id);

    struct event_base* io_event_base = evconnlistener_get_base(listener);
    struct bufferevent* buf_event = bufferevent_socket_new(io_event_base, fd, BEV_OPT_CLOSE_ON_FREE);
    if (NULL == buf_event)
    {
//        LOG_ERROR(
//                "Failed to create socket buffer event! For " << strerror(errno));
        return;
    }

    bufferevent_setcb(buf_event, OnClientDataComing, NULL, OnClientError, io_thread);
    bufferevent_setwatermark(buf_event, EV_READ, 4, 0);

    if (bufferevent_enable(buf_event, EV_READ) != 0)
    {
//        LOG_ERROR(
//                "Failed to enable socket buffer event for reading! For " << strerror(errno));
        bufferevent_free(buf_event);
        return;
    }

//    TClientSocketCtx clientSocketCtx;
//    clientSocketCtx.Reset();
//
//    clientSocketCtx._socketFD = fd;
//    clientSocketCtx._createTime = time(NULL);
//    clientSocketCtx._bufEvent = buf_event;
//
//    ioEventSink->OnClientConnected(&clientSocketCtx, sizeof(clientSocketCtx));
}

void OnListenError(struct evconnlistener* listener, void* arg)
{
    evutil_socket_t fd = evconnlistener_get_fd(listener);
    LOG4CPLUS_ERROR(g_test_logger, "failed to listen on socket fd [" << fd << "]: "
        << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
    evconnlistener_free(listener);
}

int SetupIOThread(IOThread* p, int listen_fd)
{
    p->listen_fd = listen_fd;
    p->io_event_base = event_base_new();

    return 0;
}

int CreateListener(IOThread* io_thread)
{
    struct evconnlistener* listener = evconnlistener_new(io_thread->io_event_base, OnAccept, io_thread,
                                                         LEV_OPT_REUSEABLE | LEV_OPT_CLOSE_ON_FREE, -1,
                                                         io_thread->listen_fd);
    if (NULL == listener)
    {
        LOG4CPLUS_ERROR(g_test_logger,
                        "failed to create listener: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        return -1;
    }

    evconnlistener_set_error_cb(listener, OnListenError);

    return 0;
}

void* IOThreadProc(void* arg)
{
    IOThread* io_thread = (IOThread*) arg;

    if (CreateListener(io_thread) != 0)
    {
        return (void*) 0;
    }

    pthread_mutex_lock(&g_io_thread_init_lock);
    ++init_count;
    pthread_cond_signal(&g_io_thread_init_cond);
    pthread_mutex_unlock(&g_io_thread_init_lock);

    event_base_loop(io_thread->io_event_base, 0);

    return (void*) 0;
}

int CreateIOThread(void* (* func)(void*), void* arg)
{
    IOThread* io_thread = (IOThread*) arg;

//    pthread_attr_t attr;
    pthread_t tid;

//    pthread_attr_init(&attr);
    if (pthread_create(&tid, NULL, func, arg) != 0)
    {
        LOG4CPLUS_ERROR(g_test_logger, "failed to create thread: " << strerror(errno));
        return -1;
    }

    io_thread->io_thread_id = tid;

    return 0;
}

int main()
{
    // 定义Logger
    g_test_logger = log4cplus::Logger::getInstance("LoggerName");

    // 定义一个控制台的Appender
    log4cplus::SharedAppenderPtr console_appender(new log4cplus::ConsoleAppender());

    // 将需要关联Logger的Appender添加到Logger上
    g_test_logger.addAppender(console_appender);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        LOG4CPLUS_ERROR(g_test_logger, "failed to create socket: " << strerror(errno));
        return -1;
    }

    if (evutil_make_socket_nonblocking(listen_fd) != 0)
    {
        LOG4CPLUS_ERROR(g_test_logger, "failed to set listen socket non blocking: "
            << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        close(listen_fd);
        return -1;
    }

    char addr[64] = "";
    snprintf(addr, sizeof(addr), "0.0.0.0:12345");

    struct sockaddr_in local_addr;
    int local_addr_len = sizeof(local_addr);

    if (evutil_parse_sockaddr_port(addr, (struct sockaddr*) &local_addr, &local_addr_len) != 0)
    {
        LOG4CPLUS_ERROR(g_test_logger,
                        "failed to parse sock addr and port: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        close(listen_fd);
        return -1;
    }

    if (bind(listen_fd, (struct sockaddr*) &local_addr, local_addr_len) != 0)
    {
        LOG4CPLUS_ERROR(g_test_logger, "failed to bind listen socket: "
            << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        close(listen_fd);
        return -1;
    }

    pthread_mutex_init(&g_io_thread_init_lock, NULL);
    pthread_cond_init(&g_io_thread_init_cond, NULL);

    int io_thread_count = 10;

    IOThread* threads = (IOThread*) malloc(sizeof(IOThread) * io_thread_count);

    for (int i = 0; i < io_thread_count; i++)
    {
        SetupIOThread(&threads[i], listen_fd);
    }

    for (int i = 0; i < io_thread_count; i++)
    {
        CreateIOThread(IOThreadProc, &threads[i]);
    }

    pthread_mutex_lock(&g_io_thread_init_lock);

    while (init_count < io_thread_count)
    {
        pthread_cond_wait(&g_io_thread_init_cond, &g_io_thread_init_lock);
    }

    pthread_mutex_unlock(&g_io_thread_init_lock);

    LOG4CPLUS_INFO(g_test_logger, "all io thread Init ok");

    while (1)
    {
        sleep(1);
    }

    free(threads);
    close(listen_fd);

    return 0;

}