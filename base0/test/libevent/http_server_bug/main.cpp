#include "http_server.h"

static int CreateHttpListenSocket(const char* listen_addr_port)
{
    int listen_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_socket_fd < 0)
    {
//        LOG_ERROR("failed to create listen socket: " << strerror(errno));
        return -1;
    }

    if (evutil_make_listen_socket_reuseable(listen_socket_fd) != 0)
    {
//        const int err = EVUTIL_SOCKET_ERROR();
//        LOG_ERROR("failed to set listen socket reusable: " << evutil_socket_error_to_string(err));
        return -1;
    }

    struct sockaddr_in local_addr;
    int local_addr_len = sizeof(local_addr);

    if (evutil_parse_sockaddr_port(listen_addr_port, (struct sockaddr*) &local_addr, &local_addr_len) != 0)
    {
//        const int err = EVUTIL_SOCKET_ERROR();
//        LOG_ERROR("failed to parse socket addr and port: " << evutil_socket_error_to_string(err) << ", errno: " << err << ", addr port: " << listen_addr_port);
        return -1;
    }

    if (bind(listen_socket_fd, (struct sockaddr*) &local_addr, local_addr_len) != 0)
    {
//        const int err = errno;
//        LOG_ERROR("failed to bind listen socket: " << evutil_socket_error_to_string(err));
        return -1;
    }

    if (listen(listen_socket_fd, 65535) != 0)
    {
//        const int err = errno;
//        LOG_ERROR("failed to listen on socket: " << evutil_socket_error_to_string(err));
        return -1;
    }

    if (evutil_make_socket_nonblocking(listen_socket_fd) != 0)
    {
//        const int err = EVUTIL_SOCKET_ERROR();
//        LOG_ERROR("failed to set listen socket non blocking: " << evutil_socket_error_to_string(err));
        return -1;
    }

    return listen_socket_fd;
}

int main(int argc, char* argv[])
{
    int listen_socket_fd = CreateHttpListenSocket("0.0.0.0:8080");
    if (listen_socket_fd < 0)
    {
        return -1;
    }

    const int N = 4;
    HttpServer http_server[N];

    for (int i = 0; i < N; ++i)
    {
        http_server[i].Initialize(listen_socket_fd);
        http_server[i].Activate();
    }

    for (int i = 0; i < N; ++i)
    {
        http_server[i].Freeze();
        http_server[i].Finalize();
    }

    return 0;
}
