#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include "event2/dns.h"
#include "log4cplus/logger.h"
#include "log4cplus/consoleappender.h"
#include "log4cplus/loggingmacros.h"

log4cplus::Logger g_test_logger;

void ReadCallback(struct bufferevent* buf_event, void* arg);
void EventCallback(struct bufferevent* buf_event, short events, void* arg);

int Connect(struct event_base* ev_base)
{
    std::string peer_addr = "www.baidu.com";
    std::string peer_port = "9999";

    std::vector<std::string> ip_vector;

    // hostname -> ip
    {
        struct addrinfo hints;

        /* obtaining address matching host */
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_CANONNAME;
        hints.ai_protocol = 0;  /* any protocol */

        struct addrinfo* result;

        int ret = getaddrinfo(peer_addr.c_str(), NULL, &hints, &result);
        if (ret != 0)
        {
            LOG4CPLUS_ERROR(g_test_logger, "getaddrinfo: " << gai_strerror(ret));
            return -1;
        }

        /* traverse the returned list and output the ip addresses */
        for (struct addrinfo* result_pointer = result; result_pointer != NULL; result_pointer = result_pointer->ai_next)
        {
            char host[128] = "";

            ret = getnameinfo(result_pointer->ai_addr, result_pointer->ai_addrlen, host, sizeof(host), NULL, 0,
                              NI_NUMERICHOST);
            if (ret != 0)
            {
                LOG4CPLUS_ERROR(g_test_logger, "error in getnameinfo: " << gai_strerror(ret));
                continue;
            }
            else
            {
                LOG4CPLUS_TRACE(g_test_logger, "IP: " << host);
                ip_vector.push_back(host);
            }
        }

        freeaddrinfo(result);
    }

    if (0 == ip_vector.size())
    {
        LOG4CPLUS_ERROR(g_test_logger, "failed to get ip for: " << peer_addr);
        return -1;
    }

    std::string peer_ip;

    if (ip_vector.size() > 1)
    {
        srand(time(NULL));
        peer_ip = ip_vector[rand() % ip_vector.size()];
    }
    else
    {
        peer_ip = ip_vector[0];
    }

    std::string peer_ip_port = peer_ip + ":" + peer_port;

    struct sockaddr_in peer_addr_in;
    int peer_addr_len = sizeof(peer_addr_in);

    if (evutil_parse_sockaddr_port(peer_ip_port.c_str(), (struct sockaddr*) &peer_addr_in, &peer_addr_len) != 0)
    {
        LOG4CPLUS_ERROR(g_test_logger, "failed to parse peer addr and port [" << peer_ip_port << "]: "
            << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()) << ", errno: " << EVUTIL_SOCKET_ERROR());
        return -1;
    }

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        LOG4CPLUS_ERROR(g_test_logger, "failed to create socket: " << strerror(errno));
        return -1;
    }

    if (evutil_make_socket_nonblocking(fd) != 0)
    {
        LOG4CPLUS_ERROR(g_test_logger,
                        "failed to set socket non blocking: " << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        return -1;
    }

    struct bufferevent* buf_event = bufferevent_socket_new(ev_base, -1, BEV_OPT_CLOSE_ON_FREE);
    if (NULL == buf_event)
    {
//        LOG_ERROR(
//                "Failed to create socket buffer event! For " << strerror(errno));
        return -1;
    }

    bufferevent_setcb(buf_event, ReadCallback, NULL, EventCallback, ev_base);

    if (bufferevent_enable(buf_event, EV_READ) != 0)
    {
//        LOG_ERROR(
//                "Failed to enable socket buffer event for reading! For " << strerror(errno));
        bufferevent_free(buf_event);
        return -1;
    }

    // 调试发现这个connect调用的行为很怪异，有时候会同步调用EventCallback，导致无限递归
    if (bufferevent_socket_connect(buf_event, (struct sockaddr*) &peer_addr_in, peer_addr_len) != 0)
    {
        LOG4CPLUS_ERROR(g_test_logger, "connect failed ------");
        bufferevent_free(buf_event);
        return -1;
    }

    return 0;
}

void ReadCallback(struct bufferevent* buf_event, void* arg)
{
    LOG4CPLUS_INFO(g_test_logger, "remote data comming");

    struct evbuffer* input = bufferevent_get_input(buf_event);

    char buf[1024] = "";

    evbuffer_remove(input, buf, 1024);

    LOG4CPLUS_INFO(g_test_logger, "recv data from remote: " << buf);

    std::string str = "I am client";
    evbuffer_add(bufferevent_get_output(buf_event), str.data(), str.size());
}

void EventCallback(struct bufferevent* buf_event, short events, void* arg)
{
    if (events & BEV_EVENT_CONNECTED)
    {
        LOG4CPLUS_INFO(g_test_logger, "remote connected");
        std::string str = "I am client";
        evbuffer_add(bufferevent_get_output(buf_event), str.data(), str.size());
    }

    const evutil_socket_t fd = bufferevent_getfd(buf_event);
    bool finished = false;

    if (events & BEV_EVENT_ERROR)
    {
        LOG4CPLUS_ERROR(g_test_logger, "error occured on client socket fd [" << fd << "]: "
            << evutil_socket_error_to_string(EVUTIL_SOCKET_ERROR()));
        finished = true;
    }

    if (events & (BEV_EVENT_EOF))
    {
        LOG4CPLUS_INFO(g_test_logger, "xxxxxxxxxxxxxclient closed ");
        finished = true;
    }

    if (events & (BEV_EVENT_TIMEOUT))
    {
        finished = true;
    }

    if (finished)
    {
        bufferevent_free(buf_event); // 这句是必须的，否则日志中会打印下面两句：
//        ERROR - error occured on client socket fd [6]: Connection refused
//        INFO - remote connected

        // 重连
        if (Connect((struct event_base*) arg) != 0)
        {
            LOG4CPLUS_ERROR(g_test_logger, "failed to reconnect");
        }
        else
        {
            LOG4CPLUS_INFO(g_test_logger, "reconnect");
        }
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

    struct event_base* ev_base = event_base_new();
    if (Connect(ev_base) != 0)
    {
        return -1;
    }

    event_base_dispatch(ev_base);
    event_base_free(ev_base);

    return 0;
}
