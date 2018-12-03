#include <string.h>
#include <unistd.h>
#include "zmq.h"

int pub()
{
    void* context = NULL;
    void* publisher = NULL;

    do
    {
        context = zmq_ctx_new();
        if (NULL == context)
        {
            break;
        }

        publisher = zmq_socket(context, ZMQ_PUB);
        if (NULL == publisher)
        {
            break;
        }

        int ret = zmq_bind(publisher, "epgm://eth0;239.192.1.1:5555");
        if (ret != 0)
        {
            printf("%s\n", zmq_strerror(errno));
            break;
        }

        sleep(10);

        char buf[64] = "";

        for (int i = 0; i < 20; ++i)
        {
            memset(buf, 0, sizeof(buf));
            const int len = sprintf(buf, "data packet with ID %d", i);

            zmq_msg_t message;
            zmq_msg_init_size(&message, len + 1);
            memcpy(zmq_msg_data(&message), buf, len + 1);

            if (-1 == zmq_msg_send(&message, publisher, 0))
            {
                zmq_msg_close(&message);
                break;
            }
            else
            {
                printf("send: %s\n", (char*) zmq_msg_data(&message));
                zmq_msg_close(&message);
            }

            sleep(1);
        }
    } while (0);

    if (publisher != NULL)
    {
        zmq_close(publisher);
    }

    if (context != NULL)
    {
        zmq_ctx_term(context);
    }

    return 0;
}

int main()
{
    pub();

//    // create the zeromq context
//    void *ctx = zctx_new();
//    void *pub = zsocket_new(ctx, ZMQ_PUB);
//    zsocket_connect(pub, "epgm://192.168.10.114;239.192.1.1:5000");
//    printf("multicast initialized\n");
//
//    while(!zctx_interrupted) {
//        zstr_sendm (pub, "testing", ZMQ_SNDMORE);
//        zstr_send(pub, "This is a test");
//    }

    return 0;
}
