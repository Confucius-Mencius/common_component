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
    if ((argc != 4))
    {
        cout << "Use: appserver.exe server_port client_ip client_port" << endl;
        return 0;
    }

    //startup
    //这里是对UDT的启动记性初始化操作  
    if (UDT::ERROR == UDT::startup())
    {
        cout << "startup: " << UDT::getlasterror().getErrorMessage() << endl;
    }
    else
    {
        cout << "startup suc..." << endl;
    }

    //socket
    //像声明一个普通的socket一样声明一个UDTSOCKET  
    UDTSOCKET serv = UDT::socket(AF_INET, SOCK_DGRAM, 0);
    if (UDT::ERROR == serv)
    {
        cout << "socket: " << UDT::getlasterror().getErrorMessage() << endl;
    }
    else
    {
        cout << "client suc..." << endl;
    }

    //声明udp socket，这里是udp的哈，不是udt
    int sersocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sersocket)
    {
        cout << "udp socket error!" << endl;
    }
    else
    {
        cout << "clientsocket suc..." << endl;
    }

    //为了能够在局域网中直接进行处理，先默认设置两个  
    sockaddr_in my_addr, client_addr;
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi(argv[1]));
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(my_addr.sin_zero), '\0', 8);
    bind(sersocket, (struct sockaddr*) &my_addr, sizeof(my_addr));

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(atoi(argv[3]));
    client_addr.sin_addr.s_addr = inet_addr(argv[2]);
    //client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(client_addr.sin_zero), '\0', 8);

    int mss = 1052;//最大传输单位
    //设置收发缓冲区大小 接收限时  和地址重用
    if (!(UDT::ERROR != (UDT::setsockopt(serv, 0, UDT_SNDBUF, new int(32000), sizeof(int)))
          && UDT::ERROR != (UDT::setsockopt(serv, 0, UDP_RCVBUF, new int(32000), sizeof(int)))
          && UDT::ERROR != (UDT::setsockopt(serv, 0, UDT_REUSEADDR, new int(1), sizeof(int)))
          && UDT::ERROR != (UDT::setsockopt(serv, 0, UDT_RENDEZVOUS, new bool(true), sizeof(bool))))
        && UDT::ERROR != (UDT::setsockopt(serv, 0, UDT_MSS, &mss, sizeof(int))))
    {
        cout << "udt socket: " << UDT::getlasterror().getErrorMessage() << endl;
        UDT::close(serv);
        return 0;
    }
    //这里是直接将udp的接口绑定在udt的接口之上，如果不这样做的话是没法使用UDT中的SOCK_DGRAM的  
    if (UDT::ERROR == UDT::bind2(serv, sersocket))
    {
        cout << "udt bind2:" << UDT::getlasterror().getErrorMessage() << endl;
        return 0;
    }
    else
    {
        cout << "bind2 suc" << endl;
    }
    //这里也是关键部分，与client端对应的connect操作，就是相互之间的打洞处理  
    if (UDT::ERROR == UDT::connect(serv, (sockaddr*) &client_addr, sizeof(client_addr)))
    {
        cout << "connect: " << UDT::getlasterror().getErrorMessage();
        UDT::close(serv);
        return 0;
    }
    else
    {
        cout << "connetc suc" << endl;
    }
    //这里已经可以正常接收了，接收从client发过来的filename，目的是用于本地的文件创建  
    char filename[100];
    if (UDT::ERROR == UDT::recvmsg(serv, filename, 100))
    {
        cout << "recv:" << UDT::getlasterror().getErrorMessage() << endl;
        return 0;
    }
    cout << "filename: " << filename << endl;

    //使用FILE进行文件操作，关于文件的相关操作这里不详述了，实在不懂的可以留言  
    FILE* fp;
    char localfile[100];
    memset(localfile, 0, sizeof(localfile));
    strcpy(localfile, "d:\\");
    strcat(localfile, filename);
    if ((fp = fopen(localfile, "w+")) == NULL)
    {
        cout << filename << " open failure!" << endl;
        return 0;
    }
    fclose(fp);
    fp = fopen(localfile, "wb");

    char data[MAXLEN];
    int len = 0, package = 0, filelen = 0;
    UDT::TRACEINFO trace;
    UDT::perfmon(serv, &trace);
    while (1)
    {
        //前面部分打开文件后，这里就是循环接收文件并保存  
        memset(data, 0, sizeof(data));
        len = UDT::recvmsg(serv, data, MAXLEN);
        filelen += len;
        //cout<<"filelen = "<<filelen<<endl;  
        if (strncmp("quit", data, 4) == 0)
        {
            cout << data << endl;
            fclose(fp);
            break;
        }
        else
        {
            package++;//record recv all the packages
        }
        fwrite(data, len, 1, fp);
    }
    fclose(fp);
    fp = fopen(localfile, "rb");
    fseek(fp, 0, SEEK_END);//move to the end
    filelen = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    cout << "filesize = " << filelen << endl;
    UDT::perfmon(serv, &trace);
    cout << "speed = " << trace.mbpsRecvRate << "Mbits/sec" << endl;
    cout << "recv all the packages: " << package << endl;

    fclose(fp);
    UDT::close(serv);
    UDT::cleanup();

    return 1;
}