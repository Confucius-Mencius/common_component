#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

//const char ip[] = "127.0.0.1";
int port = 6789;

int main(int argc, char** argv)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        return -1;
    }

    struct sockaddr_in local_addr;
    bzero(&local_addr, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(port);
    socklen_t sin_len = sizeof(local_addr);

    if (bind(fd, (struct sockaddr*) &local_addr, sizeof(local_addr)) < 0)
    {
        close(fd);
        return -1;
    }

    char buf[64];

    while (1)
    {
        memset(buf, 0, sizeof(buf));

        ssize_t n = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr*) &local_addr, &sin_len);
        if (n < 0)
        {
            // log
            printf("1\n");
        }

        printf("recv: %s\n", buf);

        if (strncmp(buf, "stop", 4) == 0)
        {
            break;
        }
    }

    close(fd);
    return 0;
}
