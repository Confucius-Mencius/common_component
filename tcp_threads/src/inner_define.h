#ifndef TCP_THREADS_SRC_INNER_DEFINE_H_
#define TCP_THREADS_SRC_INNER_DEFINE_H_

namespace tcp
{
struct NewConnCtx
{
    int client_sock_fd;
    char client_ip[INET_ADDRSTRLEN];
    unsigned short client_port;

    NewConnCtx()
    {
        client_sock_fd = -1;
        client_ip[0] = '\0';
        client_port = 0;
    }
};
}

#endif // TCP_THREADS_SRC_INNER_DEFINE_H_
