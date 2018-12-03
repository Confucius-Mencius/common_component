/*
 * lws-minimal-ws-client
 *
 * Copyright (C) 2018 Andy Green <andy@warmcat.com>
 *
 * This file is made available under the Creative Commons CC0 1.0
 * Universal Public Domain Dedication.
 *
 * This demonstrates the a minimal ws client using lws.
 *
 * It connects to https://libwebsockets.org/ and makes a
 * wss connection to the dumb-increment protocol there.  While
 * connected, it prints the numbers it is being sent by
 * dumb-increment protocol.
 */

#include <libwebsockets.h>
#include <string.h>
#include <signal.h>
#include <iostream>

static int interrupted, rx_seen, test;
static struct lws* client_wsi;

char msg[1024] = "";
int msg_len = 0;
int m;

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
callback_dumb_increment(struct lws* wsi, enum lws_callback_reasons reason,
                        void* user, void* in, size_t len)
{
    std::cout << "client_wsi: " << client_wsi << ", wsi: " << wsi << ", reason: " << reason << ", user: " << user
        << ", in: " << in << ", len: " << len << std::endl;

    const struct lws_protocols* protocol = lws_get_protocol(wsi);
    if (protocol != NULL)
    {
        std::cout << protocol->name << ", " << protocol->per_session_data_size
            << ", " << protocol->rx_buffer_size << ", " << protocol->id << ", "
            << protocol->tx_packet_size << std::endl;
    }

    switch (reason)
    {
        case LWS_CALLBACK_WSI_DESTROY:
            std::cout << "LWS_CALLBACK_WSI_DESTROY" << std::endl;
            break;

        case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
            std::cout << "LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH" << std::endl;
            dump_handshake_info(client_wsi);
            break;

        case LWS_CALLBACK_GET_THREAD_ID:
            std::cout << "LWS_CALLBACK_GET_THREAD_ID" << std::endl;
            break;

        case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
            std::cout << "LWS_CALLBACK_EVENT_WAIT_CANCELLED" << std::endl;
            break;

        case LWS_CALLBACK_PROTOCOL_INIT:
            std::cout << "LWS_CALLBACK_PROTOCOL_INIT" << std::endl;
            break;

        case LWS_CALLBACK_PROTOCOL_DESTROY:
            std::cout << "LWS_CALLBACK_PROTOCOL_DESTROY" << std::endl;
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

        case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
            std::cout << "LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER" << std::endl;
            break;

            /* because we are protocols[0] ... */
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            std::cout << "LWS_CALLBACK_CLIENT_CONNECTION_ERROR" << std::endl;
            lwsl_err("CLIENT_CONNECTION_ERROR: %s. errno: %d\n",
                     in ? (char*) in : "(null)", errno); // 这里的in表示为什么连接失败
            client_wsi = NULL;
            break;

        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            // 到服务器的连接建立成功
            std::cout << "LWS_CALLBACK_CLIENT_ESTABLISHED" << std::endl;
            lwsl_user("%s: established\n", __func__);
            std::cout << "LWS_PRE: " << LWS_PRE << std::endl;

            for (int i = LWS_PRE; i < (int) sizeof(msg) - 2; ++i)
            {
                msg[i] = 'a';
            }

            msg[sizeof(msg) - 1] = '\0'; // 最后一个是结束符

            lws_write(wsi, (unsigned char*) msg + LWS_PRE, sizeof(msg) - 1 - LWS_PRE,
                      LWS_WRITE_TEXT);

            break;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            std::cout << "LWS_CALLBACK_CLIENT_WRITEABLE" << std::endl;
            m = lws_write(wsi, (unsigned char*) msg + LWS_PRE, msg_len,
                          LWS_WRITE_TEXT);
            if (m < (int) msg_len)
            {
                lwsl_err("ERROR %d writing to ws\n", m);
                return -1;
            }

            msg_len = 0;
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            // 收到服务器发来的数据，应该是一个完整的帧？？
            std::cout << "LWS_CALLBACK_CLIENT_RECEIVE" << std::endl;
            std::cout << "LWS_PRE: " << LWS_PRE << std::endl;
            lwsl_user("RX: %s\n", (const char*) in);
            rx_seen++;
            if (test && rx_seen == 10)
            {
                interrupted = 1;
            }

            if (len > sizeof(msg) - 1 - LWS_PRE)
            {
                break;
            }

            msg_len = len;
            memcpy((char*) msg + LWS_PRE, in, len);
            msg[LWS_PRE + len] = '\0';

            // 开启会触发一个write事件
//            lws_callback_on_writable(client_wsi);
            break;

        case LWS_CALLBACK_CLIENT_CLOSED:
            std::cout << "LWS_CALLBACK_CLIENT_CLOSED" << std::endl;
            client_wsi = NULL;
            break;

        default:
            break;
    }

    return lws_callback_http_dummy(wsi, reason, user, in, len);
}

static const struct lws_protocols protocols[] =
    {
        {
            "lws-minimal",
            callback_dumb_increment,
                     0,
                        0,
                           0, NULL, 0
        },
        {NULL, NULL, 0, 0, 0, NULL, 0}
    };

static void
sigint_handler(int sig)
{
    interrupted = 1;
}

int main(int argc, const char** argv)
{
    bool use_ssl = false;
    struct lws_context_creation_info info;
    struct lws_client_connect_info connect_info;
    struct lws_context* context;
    const char* p;
    int n = 0, logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE
    /* for LLL_ verbosity above NOTICE to be built into lws, lws
     * must have been configured with -DCMAKE_BUILD_TYPE=DEBUG
     * instead of =RELEASE */
    /* | LLL_INFO */ /* | LLL_PARSER */ /* | LLL_HEADER */
    /* | LLL_EXT */ /* | LLL_CLIENT */ /* | LLL_LATENCY */
    /* | LLL_DEBUG */;

    signal(SIGINT, sigint_handler);
    if ((p = lws_cmdline_option(argc, argv, "-d")))
    {
        logs = atoi(p);
    }

    test = !!lws_cmdline_option(argc, argv, "-t");

    lws_set_log_level(logs, NULL);
    lwsl_user("LWS minimal ws client rx [-d <logs>] [--h2] [-t (test)]\n");

    memset(&info, 0, sizeof info); /* otherwise uninitialized garbage */
    if (use_ssl)
    {
        info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    }

    info.port = CONTEXT_PORT_NO_LISTEN; /* we do not run any server */
    info.protocols = protocols;
#if defined(LWS_WITH_MBEDTLS)
    /*
     * OpenSSL uses the system trust store.  mbedTLS has to be told which
     * CA to trust explicitly.
     */
    info.client_ssl_ca_filepath = "./libwebsockets.org.cer";
#endif

    context = lws_create_context(&info);
    if (!context)
    {
        lwsl_err("lws init failed\n");
        return 1;
    }

    memset(&connect_info, 0, sizeof connect_info); /* otherwise uninitialized garbage */
    connect_info.context = context;
    connect_info.port = 7681;
    connect_info.address = "127.0.0.1";
    connect_info.path = "/";
    connect_info.host = connect_info.address;
    connect_info.origin = connect_info.address;
    if (use_ssl)
    {
        connect_info.ssl_connection = LCCSCF_USE_SSL;
    }

    connect_info.protocol = protocols[0].name; /* "dumb-increment-protocol" */
    connect_info.pwsi = &client_wsi;

    if (lws_cmdline_option(argc, argv, "--h2"))
    {
        connect_info.alpn = "h2";
    }

    lws_client_connect_via_info(&connect_info);

    while (n >= 0 && client_wsi && !interrupted)
    {
        n = lws_service(context, 1000);
    }

    lws_context_destroy(context);

    lwsl_user("Completed %s\n", rx_seen > 10 ? "OK" : "Failed");

    return rx_seen > 10;
}
