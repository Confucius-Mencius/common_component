#ifndef WS_THREADS_SRC_NEW_CONN_H_
#define WS_THREADS_SRC_NEW_CONN_H_

#include <netinet/in.h>

namespace ws
{
enum ConnType
{
    CONN_TYPE_MIN = 0,
    CONN_TYPE_WS = CONN_TYPE_MIN,
    CONN_TYPE_WSS,
    CONN_TYPE_HTTP,
    CONN_TYPE_HTTPS,
    CONN_TYPE_MAX
};

struct NewConnCtx
{
    char client_ip[INET_ADDRSTRLEN];
    unsigned short client_port;
    int client_sock_fd;
    ConnType conn_type;

    NewConnCtx()
    {
        client_ip[0] = '\0';
        client_port = 0;
        client_sock_fd = -1;
        conn_type = CONN_TYPE_MAX;
    }
};
}

#endif // WS_THREADS_SRC_NEW_CONN_H_
