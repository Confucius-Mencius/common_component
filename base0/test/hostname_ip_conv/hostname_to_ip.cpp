/* hostname_to_ip hostname
 * OR
 * hostname_to_ip ip
 * */
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s hostname\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    struct addrinfo hints;
    struct addrinfo* result, * result_pointer;
    int ret;
    /* obtaining address matching host */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_CANONNAME;
    hints.ai_protocol = 0;  /* any protocol */

//      ret = getaddrinfo(argv[1], NULL, &hints, &result);                                                                                                            
    ret = getaddrinfo(argv[1], NULL, &hints, &result);
    if (ret != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
        exit(EXIT_FAILURE);
    }
    /* traverse the returned list and output the ip addresses */
    for (result_pointer = result; result_pointer != NULL; result_pointer = result_pointer->ai_next)
    {
        char hostname[1025] = "";
        ret = getnameinfo(result_pointer->ai_addr, result_pointer->ai_addrlen, hostname, sizeof(hostname), NULL, 0,
                          NI_NUMERICHOST);
        if (ret != 0)
        {
            fprintf(stderr, "error in getnameinfo: %s \n", gai_strerror(ret));
            continue;
        }
        else
        {
            printf("IP: %s \n", hostname);
        }
    }
    freeaddrinfo(result);
    exit(EXIT_SUCCESS);
}
