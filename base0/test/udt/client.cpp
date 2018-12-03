#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include "udt.h"

using namespace std;

#define MAXLEN 4096

int main(int argc, char* argv[])
{
    if ((argc != 5))
    {
        cout << "Usage: appclient.exe client_port server_ip server_port local_filename" << endl;
        return 0;
    }

    //startup
    if (UDT::ERROR == UDT::startup())
    {
        cout << "startup: " << UDT::getlasterror().getErrorMessage() << endl;
    }
    else
    {
        cout << "startup suc..." << endl;
    }

    //Initialize the UDT library
    UDTSOCKET client = UDT::socket(AF_INET, SOCK_DGRAM, 0);
    if (UDT::ERROR == client)
    {
        cout << "socket: " << UDT::getlasterror().getErrorMessage() << endl;
    }
    else
    {
        cout << "client suc..." << endl;
    }

    //声明udp socket
    int clientsocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == clientsocket)
    {
        cout << "udp socket error!" << endl;
    }
    else
    {
        cout << "clientsocket suc..." << endl;
    }

    sockaddr_in serv_addr, my_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[3]));
    serv_addr.sin_addr.s_addr = inet_addr(argv[2]);
    memset(&(serv_addr.sin_zero), '\0', 8);

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi(argv[1]));
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), '\0', 8);
    bind(clientsocket, (struct sockaddr*) &my_addr, sizeof(my_addr));

    int mss = 1052;//最大传输单位
    //设置收发缓冲区大小 接收限时  和地址重用
    if (!(UDT::ERROR != (UDT::setsockopt(client, 0, UDT_SNDBUF, new int(32000), sizeof(int)))
          && UDT::ERROR != (UDT::setsockopt(client, 0, UDP_RCVBUF, new int(32000), sizeof(int)))
          && UDT::ERROR != (UDT::setsockopt(client, 0, UDT_REUSEADDR, new int(1), sizeof(int)))
          && UDT::ERROR != (UDT::setsockopt(client, 0, UDT_RENDEZVOUS, new bool(true), sizeof(bool))))
        && UDT::ERROR != (UDT::setsockopt(client, 0, UDT_MSS, &mss, sizeof(int))))
    {
        cout << "udt socket: " << UDT::getlasterror().getErrorMessage() << endl;
        UDT::close(client);
        return 0;
    }

    if (UDT::ERROR == UDT::bind2(client, clientsocket))
    {
        cout << "udt bind2:" << UDT::getlasterror().getErrorMessage() << endl;
        return 0;
    }
    else
    {
        cout << "bind2 suc" << endl;
    }

    // connect to the server, implict bind
    if (UDT::ERROR == UDT::connect(client, (sockaddr*) &serv_addr, sizeof(serv_addr)))
    {
        cout << "connect: " << UDT::getlasterror().getErrorMessage();
        UDT::close(client);
        return 0;
    }
    else
    {
        cout << "connect suc" << endl;
    }

    char* hello = argv[4];
    if (UDT::ERROR == UDT::sendmsg(client, hello, strlen(hello) + 1, -1, true))
    {
        cout << "send: " << UDT::getlasterror().getErrorMessage();
        return 0;
    }

    FILE* fp;
    fp = fopen(hello, "rb");
    fseek(fp, 0, SEEK_END);
    int filesize = ftell(fp);
    //rewind(fp);//移动到头部
    fseek(fp, 0, SEEK_SET);
    static int filepos = 0;//记录文件偏移量
    cout << "filesize = " << filesize << endl;

    char data[MAXLEN + 1];
    int len = 0, package = 0;
    UDT::TRACEINFO trace;
    UDT::perfmon(client, &trace);
    while (1)
    {
        memset(data, 0, sizeof(data));
        fread(data, MAXLEN, 1, fp);
        if (filesize >= MAXLEN)
        {
            len = UDT::sendmsg(client, data, MAXLEN, -1, true);
            if (len < 0)
            {
                cout << "send failure!!\n" << endl;
                break;
            }
            filesize -= MAXLEN;
            package++;//record send all the packages
        }
        else
        {
            len = UDT::sendmsg(client, data, filesize, -1, true);
            if (len < 0)
            {
                cout << "send failure!!\n" << endl;
                break;
            }
            package++;//record recv all the packages
            char quit[] = "quit";
            if (UDT::ERROR == UDT::sendmsg(client, quit, strlen(quit) + 1, -1, true))
            {
                cout << "send quit error" << endl;
                fclose(fp);
                break;
            }
            break;
        }
    }
    UDT::perfmon(client, &trace);
    cout << "speed = " << trace.mbpsSendRate << "Mbits/sec" << endl;
    cout << "send all the packages: " << package << endl;

    fclose(fp);
    UDT::close(client);
    UDT::close(clientsocket);
    UDT::cleanup();

    return 1;
}