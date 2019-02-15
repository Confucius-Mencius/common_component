#ifndef WS_THREADS_SRC_NEW_CONN_H_
#define WS_THREADS_SRC_NEW_CONN_H_

#include <netinet/in.h>

namespace ws
{
struct NewConnCtx
{
    char client_ip[INET_ADDRSTRLEN];
    unsigned short client_port;
    int client_sock_fd;

    NewConnCtx()
    {
        client_ip[0] = '\0';
        client_port = 0;
        client_sock_fd = -1;
    }
};
}

#endif // WS_THREADS_SRC_NEW_CONN_H_
