/*
* lws-minimal-ws-server
*
* Copyright (C) 2018 Andy Green <andy@warmcat.com>
*
* This file is made available under the Creative Commons CC0 1.0
* Universal Public Domain Dedication.
*
* This demonstrates the most minimal http server you can make with lws,
* with an added websocket chat server.
*
* To keep it simple, it serves stuff in the subdirectory "./mount-origin" of
* the directory it was started in.
* You can change that by changing mount.origin.
*/

#include <libwebsockets.h>
#include <string.h>
#include <signal.h>

#define LWS_PLUGIN_STATIC
//#include "protocol_lws_minimal.c"
/*
* ws protocol handler plugin for "lws-minimal"
*
* Copyright (C) 2010-2018 Andy Green <andy@warmcat.com>
*
* This file is made available under the Creative Commons CC0 1.0
* Universal Public Domain Dedication.
*
* This version holds a single message at a time, which may be lost if a new
* message comes.  See the minimal-ws-server-ring sample for the same thing
* but using an lws_ring ringbuffer to hold up to 8 messages at a time.
*/

#if !defined (LWS_PLUGIN_STATIC)
#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#endif

#include <string.h>
#include <iostream>

/* one of these created for each message */

struct msg
{
    unsigned char* payload; /* is malloc'd */
    size_t len;
};

/* one of these is created for each client connecting to us */
struct per_session_data__minimal
{
    struct per_session_data__minimal* pss_list;  /*下一个客户端结点*/
    struct lws* wsi; /*客户端连接句柄*/
    int last; /* the last message number we sent */ /*当前接收到的消息编号*/
};

/* one of these is created for each vhost our protocol is used with */
struct per_vhost_data__minimal
{
    struct lws_context* context;
    struct lws_vhost* vhost; /*服务器，可由vhost与protocol获取该结构体*/
    const struct lws_protocols* protocol; /*使用的协议*/

    struct per_session_data__minimal* pss_list; /* linked-list of live pss*/ /*客户端链表*/

    struct msg amsg; /* the one pending message... */ /*接收到的消息，缓存大小为一条数据*/
    int current; /* the current message number we are caching */ /*当前消息编号，用来同步所有客户端的消息*/
};

/* destroys the message when everyone has had a copy of it */

static void
__minimal_destroy_message(void* _msg)
{
    struct msg* msg = (struct msg*) _msg;

    free(msg->payload);
    msg->payload = NULL;
    msg->len = 0;
}

/*
 * this is just an example of parsing handshake headers, you don't need this
 * in your code unless you will filter allowing connections by the header
 * content
 */
void
dump_handshake_info(struct lws* wsi)
{
    int n = 0, len;
    char buf[256];
    const unsigned char* c;

    do
    {
        c = lws_token_to_string(lws_token_indexes(n));
        if (!c)
        {
            n++;
            continue;
        }

        len = lws_hdr_total_length(wsi, (lws_token_indexes) n);
        if (!len || len > int(sizeof(buf) - 1))
        {
            n++;
            continue;
        }

        lws_hdr_copy(wsi, buf, sizeof buf, (lws_token_indexes) n);
        buf[sizeof(buf) - 1] = '\0';

        fprintf(stderr, "    %s = %s\n", (char*) c, buf);
        n++;
    } while (c);
}

static int
callback_minimal(struct lws* wsi, enum lws_callback_reasons reason,
                 void* user, void* in, size_t len)
{
    std::cout << "wsi: " << wsi << ", reason: " << reason << ", user: " << user << ", in: " << in << ", len: " << len
        << std::endl;

    struct per_session_data__minimal* pss =
        (struct per_session_data__minimal*) user;

    /*由vhost与protocol还原lws_protocol_vh_priv_zalloc申请的结构*/
    struct per_vhost_data__minimal* vhd =
        (struct per_vhost_data__minimal*)
            lws_protocol_vh_priv_get(lws_get_vhost(wsi),
                                     lws_get_protocol(wsi));

    const struct lws_protocols* protocol = lws_get_protocol(wsi);
    if (protocol != NULL)
    {
        std::cout << protocol->name << ", " << protocol->per_session_data_size
            << ", " << protocol->rx_buffer_size << ", " << protocol->id << ", "
            << protocol->tx_packet_size << std::endl;
    }

    int m;

    switch (reason)
    {
        case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
            std::cout << "LWS_CALLBACK_FILTER_NETWORK_CONNECTION" << std::endl;
            break;

        case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
            std::cout << "LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED" << std::endl;
            break;

        case LWS_CALLBACK_WSI_CREATE:
            std::cout << "LWS_CALLBACK_WSI_CREATE" << std::endl;
            break;

        case LWS_CALLBACK_ADD_POLL_FD:
            std::cout << "LWS_CALLBACK_ADD_POLL_FD" << std::endl;
            break;

        case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
            std::cout << "LWS_CALLBACK_CHANGE_MODE_POLL_FD" << std::endl;
            break;

        case LWS_CALLBACK_LOCK_POLL:
            std::cout << "LWS_CALLBACK_LOCK_POLL" << std::endl;
            break;

        case LWS_CALLBACK_UNLOCK_POLL:
            std::cout << "LWS_CALLBACK_UNLOCK_POLL" << std::endl;
            break;

        case LWS_CALLBACK_DEL_POLL_FD:
            std::cout << "LWS_CALLBACK_DEL_POLL_FD" << std::endl;
            break;

        case LWS_CALLBACK_WSI_DESTROY:
            std::cout << "LWS_CALLBACK_WSI_DESTROY" << std::endl;
            break;

        case LWS_CALLBACK_PROTOCOL_DESTROY:
            std::cout << "LWS_CALLBACK_PROTOCOL_DESTROY" << std::endl;
            break;

        case LWS_CALLBACK_GET_THREAD_ID:
            std::cout << "LWS_CALLBACK_GET_THREAD_ID" << std::endl;
            break;

            /*初始化*/
            // wsi=0x7fffffffd3f0, reason=LWS_CALLBACK_PROTOCOL_INIT, user=0x0, in=0x0, len=0
            // 服务启动的时候就会初始化，初始化的时候，上面的pss和vhd都为null。
        case LWS_CALLBACK_PROTOCOL_INIT:
            std::cout << "LWS_CALLBACK_PROTOCOL_INIT" << std::endl;

            /*申请内存*/
            vhd = (struct per_vhost_data__minimal*) lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
                                                                                lws_get_protocol(wsi),
                                                                                sizeof(struct per_vhost_data__minimal));
            vhd->context = lws_get_context(wsi);
            vhd->protocol = lws_get_protocol(wsi);
            vhd->vhost = lws_get_vhost(wsi);

            // 初始化完毕的结果：
//            (gdb) p vhd
//            $2 = (per_vhost_data__minimal *) 0x615bb0
//            (gdb) p *vhd
//            $3 = {context = 0x614ca0, vhost = 0x615260, protocol = 0x602158 <protocols+56>, pss_list = 0x0, amsg = {payload = 0x0, len = 0}, current = 0}
//            (gdb) p vhd->protocol
//            $4 = (const lws_protocols *) 0x602158 <protocols+56>
//            (gdb) p *vhd->protocol
//            $5 = {name = 0x40194d "lws-minimal", callback = 0x4011c3 <callback_minimal(lws*, lws_callback_reasons, void*, void*, size_t)>, per_session_data_size = 24, rx_buffer_size = 128, id = 0, user = 0x0, tx_packet_size = 0}

            break;

        case LWS_CALLBACK_EVENT_WAIT_CANCELLED: // 不用处理
            // 这里的wsi与init的不一样。所有的LWS_CALLBACK_EVENT_WAIT_CANCELLED事件的wsi都是一个
            std::cout << "LWS_CALLBACK_EVENT_WAIT_CANCELLED" << std::endl;
            break;

            // 在LWS_CALLBACK_ESTABLISHED之前，依次会收到下面2个事件，可以不用处理。这里打出来理解流程
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
            /*
             * this just demonstrates how to use the protocol filter. If you won't
             * study and reject connections based on header content, you don't need
             * to handle this callback
             */
            std::cout << "LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION" << std::endl;
            dump_handshake_info(wsi);
            /* you could return non-zero here and kill the connection */
            break;
        case LWS_CALLBACK_ADD_HEADERS:
            std::cout << "LWS_CALLBACK_ADD_HEADERS" << std::endl;

            break;

            /*建立连接，将客户端放入客户端链表*/
            // 当有新连接来时，会收到几条“LWS_CALLBACK_EVENT_WAIT_CANCELLED，这个事件是wsi都一样”，
            // 然后是LWS_CALLBACK_ESTABLISHED，接着是LWS_CALLBACK_SERVER_WRITEABLE，这两个事件的wsi是一样的，标示了这个客户端连接
        case LWS_CALLBACK_ESTABLISHED:
            std::cout << "LWS_CALLBACK_ESTABLISHED" << std::endl;

            /* add ourselves to the list of live pss held in the vhd */
            lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
            pss->wsi = wsi;
            pss->last = vhd->current;
            break;

        case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
            // 在LWS_CALLBACK_CLOSED之前会有这个事件，收到的数据长度为2(0xE903)
            /*
             * this just demonstrates how to handle
             * LWS_CALLBACK_WS_PEER_INITIATED_CLOSE and extract the peer's close
             * code and auxiliary data.  You can just not handle it if you don't
             * have a use for this.
             */
            std::cout << "LWS_CALLBACK_WS_PEER_INITIATED_CLOSE" << std::endl;

            for (int n = 0; n < (int) len; n++)
            {
                lwsl_notice(" %d: 0x%02X\n", n,
                            ((unsigned char*) in)[n]);
            }
            break;

            /*连接关闭，将客户端从链表中移除*/
            // 关闭客户端时，会收到LWS_CALLBACK_CLOSED事件，wsi标示了这个客户端连接。但是连接断开时不一定会触发这个事件？？
        case LWS_CALLBACK_CLOSED:
            std::cout << "LWS_CALLBACK_CLOSED" << std::endl;

            /* remove our closing pss from the list of live pss */
            lws_ll_fwd_remove(struct per_session_data__minimal, pss_list,
                              pss, vhd->pss_list);
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
//        表示wsi对应的ws连接当前处于可写状态，即：可发送数据至客户端。
            std::cout << "LWS_CALLBACK_SERVER_WRITEABLE" << std::endl;
            std::cout << "LWS_PRE: " << LWS_PRE << std::endl;

            if (!vhd->amsg.payload)
            {
                std::cout << "payload is null" << std::endl;
                break;
            }

            std::cout << "last msg num: " << pss->last << ", current: " << vhd->current << std::endl;

            if (pss->last == vhd->current)
            {
                break;
            }

            /* notice we allowed for LWS_PRE in the payload already */
            m = lws_write(wsi, (unsigned char*) vhd->amsg.payload + LWS_PRE, vhd->amsg.len,
                          LWS_WRITE_TEXT);
            if (m < (int) vhd->amsg.len)
            {
                lwsl_err("ERROR %d writing to ws\n", m);
                return -1;
            }

            pss->last = vhd->current;
            break;

            // 客户端发送数据时，会触发LWS_CALLBACK_RECEIVE，wsi与连接时一样
//            表示ＷＳ服务端收到客户端发送过来的一帧完整数据，此时表1中的in表示收到的数据，len表示收到的数据长度。
//            需要注意的是：指针in的回收、释放始终由LWS框架管理，只要出了回调函数，该空间就会被LWS框架回收。因此，开发者若想将接收的数据进行转发，则必须对该数据进行拷贝。
        case LWS_CALLBACK_RECEIVE:
            std::cout << "LWS_CALLBACK_RECEIVE" << std::endl;
            std::cout << "LWS_PRE: " << LWS_PRE << ", lws_is_final_fragment(wsi): " << lws_is_final_fragment(wsi) << std::endl;
            lwsl_user("RX: %s\n", (const char*) in);

            if (vhd->amsg.payload)
            {
                __minimal_destroy_message(&vhd->amsg);
            }

            vhd->amsg.len = len;
            /* notice we over-allocate by LWS_PRE */
            vhd->amsg.payload = (unsigned char*) malloc(LWS_PRE + len + 1);
            if (!vhd->amsg.payload)
            {
                lwsl_user("OOM: dropping\n");
                break;
            }

            memcpy((char*) vhd->amsg.payload + LWS_PRE, in, len); // TODO LWS_PRE是干嘛的？
            vhd->amsg.payload[LWS_PRE + len] = '\0';
            vhd->current++;
            std::cout << "current msg num: " << vhd->current << std::endl;

            /*
             * let everybody know we want to write something on them
             * as soon as they are ready
             */
            /*遍历所有的客户端，将数据放入写入回调*/
            lws_start_foreach_llp(struct per_session_data__minimal **,
                                  ppss, vhd->pss_list)
                    {
                        lws_callback_on_writable((*ppss)->wsi); // 这个调用会触发LWS_CALLBACK_SERVER_WRITEABLE，wsi时这个客户端连接的wsi
                    }
            lws_end_foreach_llp(ppss, pss_list);
            break;

        default:
            break;
    }

    return 0;
}

int user;

// 下列第三个指定了回调时user指向的内存块的大小，这个内存由lws库管理。第四个是max frame size(recv buffer size)，超过这个大小的消息会分帧发送和接收，所以需要应用自己组包.
// 设置为0是什么含义？不分帧吗？
#define LWS_PLUGIN_PROTOCOL_MINIMAL \
    { \
        "lws-minimal", \
        callback_minimal, \
        sizeof(struct per_session_data__minimal), \
        128, \
        0, NULL, 0 \
    }

#if !defined (LWS_PLUGIN_STATIC)

/* boilerplate needed if we are built as a dynamic plugin */

static const struct lws_protocols protocols[] =
{
    LWS_PLUGIN_PROTOCOL_MINIMAL
};

LWS_EXTERN LWS_VISIBLE int
init_protocol_minimal(struct lws_context *context,
                      struct lws_plugin_capability *c)
{
    if (c->api_magic != LWS_PLUGIN_API_MAGIC)
    {
        lwsl_err("Plugin API %d, library API %d", LWS_PLUGIN_API_MAGIC,
                 c->api_magic);
        return 1;
    }

    c->protocols = protocols;
    c->count_protocols = LWS_ARRAY_SIZE(protocols);
    c->extensions = NULL;
    c->count_extensions = 0;

    return 0;
}

LWS_EXTERN LWS_VISIBLE int
destroy_protocol_minimal(struct lws_context *context)
{
    return 0;
}
#endif

static struct lws_protocols protocols[] =
    {
        /*http服务器库中已做实现，直接使用lws_callback_http_dummy即可*/
//    {"http", lws_callback_http_dummy, 0, 0}, // 开启后同时支持ws和http，如果只支持ws就不要
        LWS_PLUGIN_PROTOCOL_MINIMAL,
        {NULL, NULL, 0, 0, 0, NULL, 0} /* 结束标志 */
    };

static int interrupted;

static const struct lws_http_mount mount =
    {
        /* .mount_next */        NULL,        /* linked-list "next" */
        /* .mountpoint */        "/",        /* mountpoint URL */
        /* .origin */
                                 "/home/sunlands/workspace/hilton/common_component/build/output/debug/test/websockets/ws/mount-origin",  /* serve from dir */
        /* .def */            "index.html",    /* default filename */
        /* .protocol */            NULL,
        /* .cgienv */            NULL,
        /* .extra_mimetypes */        NULL,
        /* .interpret */        NULL,
        /* .cgi_timeout */        0,
        /* .cache_max_age */        0,
        /* .auth_mask */        0,
        /* .cache_reusable */        0,
        /* .cache_revalidate */        0,
        /* .cache_intermediaries */    0,
        /* .origin_protocol */        LWSMPRO_FILE,    /* files in a dir */
        /* .mountpoint_len */        1,        /* char count */
        /* .basic_auth_login_file */    NULL,
    };

void sigint_handler(int sig)
{
    interrupted = 1;
}

// 实现一个简单的聊天室，即当一个页面发送消息时，所有的连接的页面都会收到该消息。
int main(int argc, const char** argv)
{
    struct lws_context_creation_info info;
    struct lws_context* context;
    const char* p;
    int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
    /* for LLL_ verbosity above NOTICE to be built into lws,
     * lws must have been configured and built with
     * -DCMAKE_BUILD_TYPE=DEBUG instead of =RELEASE */
    /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
    /* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
    /* | LLL_DEBUG */;

    signal(SIGINT, sigint_handler);

    if ((p = lws_cmdline_option(argc, argv, "-d")))
    {
        logs = atoi(p);
    }

    lws_set_log_level(logs, NULL);
    lwsl_user("LWS minimal ws server | visit http://localhost:7681\n");

    memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
    info.port = 7681;

    /*设置http服务器的配置*/
//    info.mounts = &mount; 开启后可以支持http访问

    /*添加协议*/
    info.protocols = protocols;

    context = lws_create_context(&info);
    if (!context)
    {
        lwsl_err("lws init failed\n");
        return 1;
    }

    while (n >= 0 && !interrupted)
    {
        n = lws_service(context, 1000);
    }

    lws_context_destroy(context);

    return 0;
}
