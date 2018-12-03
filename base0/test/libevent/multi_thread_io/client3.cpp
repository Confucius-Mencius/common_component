#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include "log4cplus/logger.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/loggingmacros.h"

log4cplus::Logger g_test_logger;

const std::string g_message(256, '1');

void WriteCallback(struct bufferevent* bev, void* ctx)
{
    LOG4CPLUS_INFO(g_test_logger, "write callback, number:" << g_message.size());
    bufferevent_write(bev, g_message.c_str(), g_message.size());
}

void EventCallback(struct bufferevent* bev, short what, void* ctx)
{
    LOG4CPLUS_INFO(g_test_logger, "on event - ");
    if (what & BEV_EVENT_TIMEOUT)
    {
        LOG4CPLUS_INFO(g_test_logger, "time out");
    }
    else if (what & BEV_EVENT_EOF)
    {
        LOG4CPLUS_INFO(g_test_logger, "connection closed");
    }
    else if (what & BEV_EVENT_ERROR)
    {
        LOG4CPLUS_INFO(g_test_logger, "event error");
    }
    else if (what & BEV_EVENT_CONNECTED)
    {
        LOG4CPLUS_INFO(g_test_logger, "connected");
        evutil_make_socket_nonblocking(bufferevent_getfd(bev));
        bufferevent_write(bev, g_message.c_str(), g_message.size());
    }
    else if (what & BEV_EVENT_READING)
    {
        LOG4CPLUS_INFO(g_test_logger, "reading");
    }
    else if (what & BEV_EVENT_WRITING)
    {
        LOG4CPLUS_INFO(g_test_logger, "writing");
    }
    else
    {
        LOG4CPLUS_INFO(g_test_logger, "others");
    }
}

int main()
{
    // 定义Logger
    g_test_logger = log4cplus::Logger::getInstance("LoggerName");

    // 定义一个控制台的Appender
    log4cplus::SharedAppenderPtr console_appender(new log4cplus::ConsoleAppender());

    // 将需要关联Logger的Appender添加到Logger上
    g_test_logger.addAppender(console_appender);

//    int fd = socket(AF_INET, SOCK_STREAM, 0);
//    if (fd < 0)
//    {
//        LOG4CPLUS_ERROR(console_logger_, "failed to create socket: " << strerror(errno));
//        return -1;
//    }
//
//    if (evutil_make_socket_nonblocking(fd) != 0)
//    {
//        LOG4CPLUS_ERROR(console_logger_, "failed to set listen socket non blocking: " <<
//                                       evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
//        close(fd);
//        return -1;
//    }

    char addr[64] = "";
    snprintf(addr, sizeof(addr), "0.0.0.0:12345");

    struct sockaddr_in remote_addr;
    int remote_addr_len = sizeof(remote_addr);

    if (evutil_parse_sockaddr_port(addr, (struct sockaddr*) &remote_addr, &remote_addr_len) != 0)
    {
        LOG4CPLUS_ERROR(g_test_logger,
                        "failed to parse sock addr and port: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        return -1;
    }
//
//    if (connect(fd, (struct sockaddr*) &remote_addr, remote_addr_len) != 0)
//    {
//        LOG4CPLUS_ERROR(console_logger_, "failed to connect to remote server: " << strerror(errno));
//        return -1;
//    }

    struct event_base* io_event_base = event_base_new();

    struct bufferevent* buf_event = bufferevent_socket_new(io_event_base, -1, BEV_OPT_CLOSE_ON_FREE);
    if (NULL == buf_event)
    {
//        LOG_ERROR(
//                "Failed to create socket buffer event! For " << strerror(errno));
        event_base_free(io_event_base);
        return -1;
    }

    bufferevent_setcb(buf_event, NULL, WriteCallback, EventCallback, NULL);

    if (bufferevent_enable(buf_event, EV_WRITE | EV_PERSIST) != 0)
    {
//        LOG_ERROR(
//                "Failed to enable socket buffer event for reading! For " << strerror(errno));
        bufferevent_free(buf_event);
        event_base_free(io_event_base);
        return -1;
    }

    bufferevent_socket_connect(buf_event, (struct sockaddr*) &remote_addr, remote_addr_len);

    event_base_dispatch(io_event_base);

//
//    std::string str = "I am Michael";
//
//    while (true)
//    {
//        send(fd, str.c_str(), str.size_(), 0);
//        sleep(5);
//    }

    return 0;
}
