#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

const char ip[] = "127.0.0.1";
int port = 6789;

int main(int argc, char** argv)
{
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        return -1;
    }

    struct sockaddr_in remote_addr;
    bzero(&remote_addr, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(ip);
    remote_addr.sin_port = htons(port);

    char buf[64] = "";

    for (int i = 0; i < 20; ++i)
    {
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "data packet with ID %d", i);

        ssize_t n = sendto(fd, buf, strlen(buf), 0, (struct sockaddr*) &remote_addr, sizeof(remote_addr));
        if (n < 0)
        {
            // log
            printf("1\n");
        }
    }

    memset(buf, 0, sizeof(buf));
    sprintf(buf, "stop");

    ssize_t n = sendto(fd, buf, strlen(buf), 0, (struct sockaddr*) &remote_addr, sizeof(remote_addr));
    if (n < 0)
    {
        // log
        printf("2\n");
    }

    close(fd);
    return 0;
}
