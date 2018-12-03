#include "zeromq_test.h"
#include "zmq.h"

static void* publisher_thread(void* args)
{
    void* context = args;
    void* publisher = zmq_socket(context, ZMQ_PUB);
    zmq_bind(publisher, "inproc://my_publisher");
    const char string[] = "Hi!";

    for (int i = 0; i < 10000; ++i)
    {
        //  Send message to all subscribers
        zmq_msg_t message;
        zmq_msg_init_size(&message, strlen(string));
        memcpy(zmq_msg_data(&message), string, strlen(string));
        int rc = zmq_msg_send(&message, publisher, 0);
        if (-1 == rc)
        {
            printf("send error");
        }
        zmq_msg_close(&message);
        printf("send ok");
    }

    // notify the subscriber to stop
    const char string_exit[] = "exit";
    zmq_msg_t message;
    zmq_msg_init_size(&message, strlen(string_exit));
    memcpy(zmq_msg_data(&message), string_exit, strlen(string_exit));
    int rc = zmq_msg_send(&message, publisher, 0);
    if (-1 == rc)
    {
        printf("send error");
    }
    zmq_msg_close(&message); // 在zmq_msg_send之后立即zmq_msg_close，消息是不会发出去的

    sleep(1);
    zmq_close(publisher);
    pthread_exit((void*) 0);
}

static void* subscriber_thread(void* args)
{
    void* context = args;
    void* subscriber = zmq_socket(context, ZMQ_SUB);

    /* Connect it to an in-process transport with the address 'my_publisher' */
    int rc = zmq_connect(subscriber, "inproc://my_publisher");
    assert(rc == 0);

    // 必须通过zmq_setsock进行设置过滤器，过滤器如果为“”， 则表示订阅所有消息，否则就只订阅开头匹配的消息
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);

    while (1)
    {
        // Receive message from server
        zmq_msg_t message;
        zmq_msg_init(&message);

        rc = zmq_msg_recv(&message, subscriber, 0);
        if (-1 == rc)
        {
            printf("recv error");
            continue;
        }

        int size = zmq_msg_size(&message);
        if (size > 0)
        {
            char* string = (char*) malloc(size + 1);
            memcpy(string, zmq_msg_data(&message), size);
            zmq_msg_close(&message);
            string[size] = 0;
            printf("Message is: %s\n", string);

            if (0 == strcmp(string, "exit"))
            {
                zmq_msg_close(&message);
                break;
            }
        }
        else
        {
            zmq_msg_close(&message);
        }
    }

    zmq_close(subscriber);
    pthread_exit((void*) 0);
}

ZeroMQTest::ZeroMQTest()
{

}

ZeroMQTest::~ZeroMQTest()
{

}

void ZeroMQTest::Test001()
{
    void* context = zmq_ctx_new();

    pthread_t pub_tid;
    pthread_t sub_tid;

    // 应该在二者建立好连接之后发布者才开始发布消息，否则会丢消息，这在消息数很小的时候很容易发现
    pthread_create(&pub_tid, NULL, publisher_thread, context);
    pthread_create(&sub_tid, NULL, subscriber_thread, context);

    pthread_join(pub_tid, NULL);
    pthread_join(sub_tid, NULL);

    zmq_ctx_term(context);
    context = NULL;
}

ADD_TEST_F(ZeroMQTest, Test001);
