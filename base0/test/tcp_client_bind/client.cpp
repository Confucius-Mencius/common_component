#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 1024

int main(int argc, char** argv)
{
    while (true)
    {
        int socketfd;
        struct sockaddr_in sockaddr, self_sockaddr;
        char sendline[MAXLINE];

        socketfd = socket(AF_INET, SOCK_STREAM, 0);

        int optval = 1;
        if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
        {
            perror("setsockopt1 error");
            return -1;
        }

        memset(&self_sockaddr, 0, sizeof(self_sockaddr));
        self_sockaddr.sin_family = AF_INET;
        self_sockaddr.sin_port = htons(0); // 由内核分配port，也可以指定一个，例如10005
        inet_pton(AF_INET, "127.0.0.1", &self_sockaddr.sin_addr);

        int ret = bind(socketfd, (struct sockaddr*) &self_sockaddr, sizeof(self_sockaddr));
        if (ret != 0)
        {
            if (EADDRINUSE == errno)
            {
//                we run out of available ports
            }
            else
            {
                printf("connect error %s errno: %d\n", strerror(errno), errno);
            }

            return -1;
        }

        memset(&sockaddr, 0, sizeof(sockaddr));
        sockaddr.sin_family = AF_INET;
        sockaddr.sin_port = htons(10004);
        inet_pton(AF_INET, "127.0.0.1", &sockaddr.sin_addr);

        if ((connect(socketfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr))) < 0)
        {
            if (EADDRNOTAVAIL == errno)
            {
//                retry
            }
            else
            {
                printf("connect error %s errno: %d\n", strerror(errno), errno);
                return -1;
            }
        }

        printf("send message to server\n");

        fgets(sendline, MAXLINE, stdin);

        if ((send(socketfd, sendline, strlen(sendline), 0)) < 0)
        {
            printf("send mes error: %s errno : %d", strerror(errno), errno);
            return -1;
        }

        close(socketfd);
    }
}
