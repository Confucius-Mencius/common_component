#include <stdio.h>
#include "zmq.h"

int sub()
{
    void* context = NULL;
    void* subscriber = NULL;

    do
    {
        context = zmq_ctx_new();
        if (NULL == context)
        {
            break;
        }

        subscriber = zmq_socket(context, ZMQ_SUB);
        if (NULL == subscriber)
        {
            break;
        }

//        终结点（endpoint）应该是一个网口跟一个分号，再跟一个多播地址，再跟一个冒号，然后是一个端口号。
//
//        一个网口可能是下面定义中的一种：
//
//        ● 由操作系统定义的网口名称
//
//        ● 分配给这个网口的主IPv4地址，以数字形式表示
//
//        网口名不应该以任何形式标准化，而且应该假定是任意的和平台相关的。在Win32平台上没有短网口名，而只有IPv4地址可以用来指定一个网口。网口部分可以省略，此时就会选择默认的网口。
//        一个多播地址是以IPv4的数字形式表示的。
//        多播组的地址是D类IP，规定是224.0.0.0-239.255.255.255。
//        对于多播地址，有几个特殊的多播地址被占用，他们是
//
//        224.0.0.1--该子网内所有的系统组。
//        224.0.0.2--该子网内所有的路由器。
//        224.0.1.1--网络实现协议NTP专用IP。
//        224.0.0.9--RIPv2专用IP
        int ret = zmq_connect(subscriber, "epgm://eth0;239.192.1.1:5555");
        if (ret != 0)
        {
            printf("%s\n", zmq_strerror(errno));
            break;
        }

        // 必须通过zmq_setsock进行设置过滤器，过滤器如果为“”， 则表示订阅所有消息，否则就只订阅开头匹配的消息
        zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);

        while (1)
        {
            // Receive message from server
            zmq_msg_t message;
            zmq_msg_init(&message);

            if (-1 == zmq_msg_recv(&message, subscriber, 0))
            {
                zmq_msg_close(&message);
                continue;
            }

            if (zmq_msg_size(&message) > 0)
            {
                printf("recv: %s\n", (char*) zmq_msg_data(&message));
                zmq_msg_close(&message);
            }
            else
            {
                zmq_msg_close(&message);
            }
        }
    } while (0);

    if (subscriber != NULL)
    {
        zmq_close(subscriber);
    }

    if (context != NULL)
    {
        zmq_ctx_term(context);
    }

    return 0;
}

int main()
{
    sub();

//    // create the zeromq context
//    void *ctx = zctx_new();
//    void *sub = zsocket_new(ctx, ZMQ_SUB);
//    zsocket_connect(sub, "epgm://192.168.10.114;239.192.1.1:5000");
//    zsocket_set_subscribe(sub, "testing");
//    printf("multicast initialized\n");
//
//    while(!zctx_interrupted) {
//        zmsg_t *msg = zmsg_new();
//        msg = zmsg_recv (sub);
//        printf("message size = %d\n" + zmsg_size(msg));
//
//        int i;
//        for (i = 0; i < zmsg_size(msg); i++) {
//            printf("Data frame %d is %s\n", i, zframe_data(zmsg_next(msg)));
//        }
//        zmsg_destroy (&msg);
//    }

    return 0;
}
