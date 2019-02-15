#include "ws_controller.h"
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "app_frame_conf_mgr_interface.h"
#include "conn.h"
#include "log_util.h"
#include "new_conn.h"
#include "thread_sink.h"

namespace ws
{
/* one of these is created for each vhost our protocol is used with */
//struct per_vhost_data
//{
//    struct lws_context* context;
//    struct lws_vhost* vhost;
//    const struct lws_protocols* protocol;
//};

/*
 * this is just an example of parsing handshake headers, you don't need this
 * in your code unless you will filter allowing connections by the header
 * content
 */
void dump_handshake_info(struct lws* wsi)
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

        lwsl_notice("    %s = %s\n", (char*) c, buf);
        n++;
    } while (c);
}

static int callback_ws(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
    (void) user;

    LOG_TRACE("wsi: " << wsi << ", reason: " << reason << ", user: " << user << ", in: " << in << ", len: " << len);

//    struct per_session_data* pss =
//        (struct per_session_data*) user;

    // 由vhost与protocol获取通过lws_protocol_vh_priv_zalloc分配的结构
//    struct per_vhost_data* vhd = (struct per_vhost_data*) lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));
//    if (vhd != NULL)
//    {
//    }

    const struct lws_protocols* protocol = lws_get_protocol(wsi);
    if (protocol != NULL)
    {
        LOG_DEBUG(protocol->name << ", " << protocol->per_session_data_size
                  << ", " << protocol->rx_buffer_size << ", " << protocol->id << ", "
                  << protocol->tx_packet_size);
    }

    switch (reason)
    {
        case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
            LOG_TRACE("LWS_CALLBACK_FILTER_NETWORK_CONNECTION");
            break;

        case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
            LOG_TRACE("LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED");
            break;

        case LWS_CALLBACK_WSI_CREATE:
            LOG_TRACE("LWS_CALLBACK_WSI_CREATE");
            break;

        case LWS_CALLBACK_ADD_POLL_FD:
            LOG_TRACE("LWS_CALLBACK_ADD_POLL_FD");
            break;

        case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
            LOG_TRACE("LWS_CALLBACK_CHANGE_MODE_POLL_FD");
            break;

        case LWS_CALLBACK_LOCK_POLL:
            LOG_TRACE("LWS_CALLBACK_LOCK_POLL");
            break;

        case LWS_CALLBACK_UNLOCK_POLL:
            LOG_TRACE("LWS_CALLBACK_UNLOCK_POLL");
            break;

        case LWS_CALLBACK_DEL_POLL_FD:
            LOG_TRACE("LWS_CALLBACK_DEL_POLL_FD");
            break;

        case LWS_CALLBACK_WSI_DESTROY:
            LOG_TRACE("LWS_CALLBACK_WSI_DESTROY");
            break;

        case LWS_CALLBACK_PROTOCOL_DESTROY:
            LOG_TRACE("LWS_CALLBACK_PROTOCOL_DESTROY");
            break;

        case LWS_CALLBACK_GET_THREAD_ID:
            LOG_TRACE("LWS_CALLBACK_GET_THREAD_ID");
            break;

        // 初始化
        case LWS_CALLBACK_PROTOCOL_INIT:
        {
            LOG_TRACE("LWS_CALLBACK_PROTOCOL_INIT");

            // 分配vhost+protocol相关的存储块
//            vhd = (struct per_vhost_data*) lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct per_vhost_data));
//            if (NULL == vhd)
//            {
//                const int err = errno;
//                LOG_ERROR("lws_protocol_vh_priv_zalloc failed, errno: " << err << ", err msg: " << strerror(err));
//                return -1;
//            }

//            vhd->context = lws_get_context(wsi);
//            vhd->vhost = lws_get_vhost(wsi);
//            vhd->protocol = lws_get_protocol(wsi);
        }
        break;

        case LWS_CALLBACK_EVENT_WAIT_CANCELLED: // 不用处理
            // 这里的wsi与init时的不一样。所有的LWS_CALLBACK_EVENT_WAIT_CANCELLED事件的wsi都是一个，是全局的
            LOG_TRACE("LWS_CALLBACK_EVENT_WAIT_CANCELLED");
            break;

        // 在LWS_CALLBACK_ESTABLISHED之前，依次会收到下面2个事件，可以不用处理。这里打出来理解流程
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        {
            /*
             * this just demonstrates how to use the protocol filter. If you won't
             * study and reject connections based on header content, you don't need
             * to handle this callback
             */
            LOG_TRACE("LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION");
            dump_handshake_info(wsi);
            /* you could return non-zero here and kill the connection */
        }
        break;

        case LWS_CALLBACK_ADD_HEADERS:
        {
            // TODO 添加额外的http头
            LOG_TRACE("LWS_CALLBACK_ADD_HEADERS");
        }
        break;

        // 连接建立成功
        // 当有新连接来时，会收到几条LWS_CALLBACK_EVENT_WAIT_CANCELLED，这个事件是wsi都一样，
        // 然后是LWS_CALLBACK_ESTABLISHED，接着是LWS_CALLBACK_SERVER_WRITEABLE，后面两个事件的wsi是一样的，标示了这个客户端连接
        case LWS_CALLBACK_ESTABLISHED:
        {
            LOG_TRACE("LWS_CALLBACK_ESTABLISHED");
            ThreadSink* thread_sink = static_cast<ThreadSink*>(lws_vhost_user(lws_get_vhost(wsi)));

            NewConnCtx new_conn_ctx;
            new_conn_ctx.client_sock_fd = lws_get_socket_fd(wsi);

            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            if (getpeername(new_conn_ctx.client_sock_fd, (struct sockaddr*) &client_addr, &addr_len) == -1)
            {
                const int err = errno;
                LOG_ERROR("getpeername failed, socket fd: " << new_conn_ctx.client_sock_fd << ", errno: " << err << ", err msg: " << strerror((err)));
                return -1;
            }

            if (NULL == evutil_inet_ntop(AF_INET, &(client_addr.sin_addr),
                                         new_conn_ctx.client_ip, sizeof(new_conn_ctx.client_ip)))
            {
                LOG_ERROR("failed to get client ip, socket fd: " << new_conn_ctx.client_sock_fd);
            }
            else
            {
                new_conn_ctx.client_port = ntohs(client_addr.sin_port);
                LOG_DEBUG("conn connected, client ip: " << new_conn_ctx.client_ip << ", port: " << new_conn_ctx.client_port
                          << ", socket fd: " << new_conn_ctx.client_sock_fd);
            }

            if (thread_sink->OnClientConnected(&new_conn_ctx) != 0)
            {
                return -1;
            }

            Conn* conn = static_cast<Conn*>(thread_sink->GetConnMgr()->GetConn(new_conn_ctx.client_sock_fd));
            if (NULL == conn)
            {
                LOG_ERROR("failed to get ws conn by socket fd: " << new_conn_ctx.client_sock_fd);
                return -1;
            }

            conn->SetWsi(wsi);
        }
        break;

        // 在LWS_CALLBACK_CLOSED之前会有这个事件，收到的数据长度为2(0xE903)
        case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
        {
            /*
             * this just demonstrates how to handle
             * LWS_CALLBACK_WS_PEER_INITIATED_CLOSE and extract the peer's close
             * code and auxiliary data.  You can just not handle it if you don't
             * have a use for this.
             */
            LOG_TRACE("LWS_CALLBACK_WS_PEER_INITIATED_CLOSE");

            for (int n = 0; n < (int) len; n++)
            {
                lwsl_notice(" %d: 0x%02X\n", n, ((unsigned char*) in)[n]);
            }
        }
        break;

        // 连接关闭
        // 关闭客户端时，会收到LWS_CALLBACK_CLOSED事件，wsi标示了这个客户端连接
        case LWS_CALLBACK_CLOSED:
        {
            LOG_TRACE("LWS_CALLBACK_CLOSED");
            ThreadSink* thread_sink = static_cast<ThreadSink*>(lws_vhost_user(lws_get_vhost(wsi)));
            const lws_sockfd_type sock_fd = lws_get_socket_fd(wsi);

            BaseConn* conn = thread_sink->GetConnMgr()->GetConn(sock_fd);
            if (NULL == conn)
            {
                LOG_ERROR("failed to get ws conn by socket fd: " << sock_fd);
                return -1;
            }

            thread_sink->OnClientClosed(conn);
        }
        break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
        {
            // 表示wsi对应的ws连接当前处于可写状态，即：可发送数据至客户端。
            LOG_TRACE("LWS_CALLBACK_SERVER_WRITEABLE");
            LOG_DEBUG("LWS_PRE: " << LWS_PRE);

            ThreadSink* thread_sink = static_cast<ThreadSink*>(lws_vhost_user(lws_get_vhost(wsi)));
            const lws_sockfd_type sock_fd = lws_get_socket_fd(wsi);

            ConnMgr* conn_mgr = thread_sink->GetConnMgr();
            Conn* conn = static_cast<Conn*>(conn_mgr->GetConn(sock_fd));
            if (NULL == conn)
            {
                LOG_ERROR("failed to get ws conn by socket fd: " << sock_fd);
                return -1;
            }

            conn->SendListData();
        }
        break;

        // 收到客户端的数据
        // 客户端发送数据时，会触发LWS_CALLBACK_RECEIVE，wsi与建立连接时一样
        // 服务端此时收到客户端发送过来的一帧完整数据，此时参数in表示收到的数据，len表示数据长度
        case LWS_CALLBACK_RECEIVE:
        {
            LOG_TRACE("LWS_CALLBACK_RECEIVE");
            LOG_DEBUG("LWS_PRE: " << LWS_PRE << ", lws_is_final_fragment(wsi): " << lws_is_final_fragment(wsi));
            LOG_DEBUG("in: " << (const char*) in << ", len: " << len);

            ThreadSink* thread_sink = static_cast<ThreadSink*>(lws_vhost_user(lws_get_vhost(wsi)));

            if (thread_sink->GetThread()->IsStopping())
            {
                LOG_WARN("in stopping status, refuse all client data");
                return -1;
            }

            const lws_sockfd_type sock_fd = lws_get_socket_fd(wsi);

            ConnMgr* conn_mgr = thread_sink->GetConnMgr();
            BaseConn* conn = conn_mgr->GetConn(sock_fd);
            if (NULL == conn)
            {
                LOG_ERROR("failed to get ws conn by socket fd: " << sock_fd);
                return -1;
            }

            if (conn_mgr->UpdateConnStatus(conn->GetConnGUID()->conn_id) != 0)
            {
                thread_sink->OnClientClosed(conn);
                return -1;
            }

            thread_sink->OnRecvClientData(conn->GetConnGUID(), in, len);
        }
        break;

        default:
            break;
    }

    return 0;
}

static struct lws_protocols protocols[] =
{
//    TODO 先调ws，再调http
//    {
//        "lws-http",    /* name */
//        callback_http,    /* callback */
//        sizeof(struct per_session_data__http),  /* per_session_data_size */
//        0,      /* max frame size / rx buffer */
//        0, NULL, 0
//    },
    {
        "lws-ws",
        callback_ws,
        0, // 指定了回调时user指向的内存块的大小，这个内存由lws库管理
        0, // max frame size(recv buffer size)，超过这个大小的消息会分帧发送和接收，所以需要应用自己组包。设置为0是什么含义？不分帧吗？
        0, NULL, 0
    },
    { NULL, NULL, 0, 0, 0, NULL, 0 } /* 结束标志 */
};

//static const struct lws_extension exts[] =
//{
//    {
//        "permessage-deflate",
//        lws_extension_callback_pm_deflate,
//        "permessage-deflate; client_no_context_takeover; client_max_window_bits"
//    },
//    {
//        "deflate-frame",
//        lws_extension_callback_pm_deflate,
//        "deflate_frame"
//    },
//    { NULL, NULL, NULL } /* 结束标志 */
//};

WSController::WSController()
{
    threads_ctx_ = NULL;
    thread_sink_ = NULL;
    ws_context_ = NULL;
    wss_context_ = NULL;
}

WSController::~WSController()
{

}

int WSController::Initialize(const ThreadsCtx* threads_ctx)
{
    threads_ctx_ = threads_ctx;

    if (CreateWSContext(false) != 0)
    {
        return -1;
    }

    if (CreateWSContext(true) != 0)
    {
        return -1;
    }

    return 0;
}

void WSController::Finalize()
{
    if (wss_context_ != NULL)
    {
        lws_context_destroy(wss_context_);
        wss_context_ = NULL;
    }

    if (ws_context_ != NULL)
    {
        lws_context_destroy(ws_context_);
        ws_context_ = NULL;
    }
}

int WSController::CreateWSContext(bool use_ssl)
{
    unsigned int opts = LWS_SERVER_OPTION_ALLOW_LISTEN_SHARE | LWS_SERVER_OPTION_LIBEVENT;
    std::string ws = "ws";
    struct lws_context_creation_info* info = &ws_info_;

    if (use_ssl)
    {
        info = &wss_info_;
        ws = "wss";
    }

    memset(info, 0, sizeof(struct lws_context_creation_info));

    if (use_ssl)
    {
        info->port = threads_ctx_->conf_mgr->GetWSSPort();
    }
    else
    {
        info->port = threads_ctx_->conf_mgr->GetWSPort();
    }

    if (0 == threads_ctx_->conf_mgr->GetWSIface().length())
    {
        info->iface = NULL; // 监听所有ip
        LOG_ALWAYS(ws << " listen addr port: 0.0.0.0:" << info->port);
    }
    else
    {
        strcpy(iface_, threads_ctx_->conf_mgr->GetWSIface().c_str());
        info->iface = iface_;

        std::string iface_ip = "";
        struct ifaddrs* ifaddr = NULL;

        if (getifaddrs(&ifaddr) != 0)
        {
            const int err = errno;
            LOG_ERROR("getifaddrs failed, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }

        char ip[16] = "";
        char netmask[16] = "";

        for (struct ifaddrs* ifp = ifaddr; ifp != NULL; ifp = ifp->ifa_next)
        {
            if (ifp->ifa_addr && ifp->ifa_addr->sa_family == AF_INET)
            {
                if (0 == strcmp(ifp->ifa_name, iface_))
                {
                    strncpy(ip, inet_ntoa(((struct sockaddr_in*)ifp->ifa_addr)->sin_addr), 16);
                    strncpy(netmask, inet_ntoa(((struct sockaddr_in*)ifp->ifa_netmask)->sin_addr), 16);
                    break;
                }
            }
        }

        freeifaddrs(ifaddr);
        LOG_ALWAYS(ws << " listen addr port: " << ip << ":" << info->port);
    }

//    /* 设置http服务器的配置 */
//    http_mount_ =
//    {
//        /* .mount_next */             NULL,        /* linked-list "next" */
//        /* .mountpoint */             "/",        /* mountpoint URL */
//        /* .origin */                 "/var/www/warmcat.com",  /* serve from dir */
//        /* .def */                    "index.html",    /* default filename */
//        /* .protocol */               NULL,
//        /* .cgienv */                 NULL,
//        /* .extra_mimetypes */        NULL,
//        /* .interpret */              NULL,
//        /* .cgi_timeout */            0,
//        /* .cache_max_age */          0,
//        /* .auth_mask */              0,
//        /* .cache_reusable */         0,
//        /* .cache_revalidate */       0,
//        /* .cache_intermediaries */   0,
//        /* .origin_protocol */        LWSMPRO_FILE,    /* files in a dir */
//        /* .mountpoint_len */         1,        /* char count */
//        /* .basic_auth_login_file */  NULL,
//        { NULL, NULL }
//    };

    info->protocols = protocols;
    info->extensions = NULL;

    if (use_ssl)
    {
        strcpy(cert_file_path_, threads_ctx_->conf_mgr->GetWSSCertificateChainFilePath().c_str());
        LOG_ALWAYS("cert file: " << cert_file_path_);

        strcpy(private_key_file_path_, threads_ctx_->conf_mgr->GetWSSPrivateKeyFilePath().c_str());
        LOG_ALWAYS("private key file: " << private_key_file_path_);

        info->ssl_cert_filepath = cert_file_path_;
        info->ssl_private_key_filepath = private_key_file_path_;
        opts |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT; // TODO app frame中的ssl init看还要不要
    }
    else
    {
        info->ssl_private_key_password = NULL; // TODO 查一下这些东西
        info->ssl_cert_filepath = NULL;
        info->ssl_private_key_filepath = NULL;
        info->ssl_ca_filepath = NULL; // TODO
        info->ssl_cipher_list = NULL; // TODO
    }

    info->gid = -1;
    info->uid = -1;
    info->user = this;
    info->options = opts;
    info->max_http_header_data = 0;
    info->max_http_header_data2 = 1024; // The max amount of header payload that can be handled in an http request TODO 配置
    info->max_http_header_pool = 0; // The max number of connections with http headers that can be processed simultaneously. 0 = allow as many ah as number of availble fds for the process
    info->count_threads = 1; // how many contexts to create in an array, 0 = 1
    info->fd_limit_per_thread = 0; // nonzero means restrict each service thread to this many fds, 0 means the default which is divide the process fd limit by the number of threads.
    info->keepalive_timeout = 60; // seconds to allow remote client to hold on to an idle HTTP/1.1 connection, 0 = 5s TODO 配置
    info->mounts = NULL;
    info->ws_ping_pong_interval = 0; // 0 for none, else interval in seconds between sending PINGs on idle websocket connections.
    info->headers = NULL; // pointer to optional linked list of per-vhost canned headers that are added to server responses
    info->reject_service_keywords = NULL; // Optional list of keywords and rejection codes + text. The keywords are checked for existing in the user agent string.
    info->client_ssl_private_key_password = NULL; // TODO 查一下这些东西
    info->client_ssl_private_key_filepath = NULL;
    info->client_ssl_cert_filepath = NULL;
    info->client_ssl_ca_filepath = NULL;
    info->client_ssl_cipher_list = NULL;

    if (use_ssl)
    {
        wss_foreign_loops_[0] = thread_sink_->GetThread()->GetThreadEvBase();
        info->foreign_loops = wss_foreign_loops_;

        wss_context_ = lws_create_context(info);
        if (NULL == wss_context_)
        {
            const int err = errno;
            LOG_ERROR("lws_create_context failed, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }
    }
    else
    {
        ws_foreign_loops_[0] = thread_sink_->GetThread()->GetThreadEvBase();
        info->foreign_loops = ws_foreign_loops_;

        ws_context_ = lws_create_context(info);
        if (NULL == ws_context_)
        {
            const int err = errno;
            LOG_ERROR("lws_create_context init failed, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }
    }

    return 0;
}
}
