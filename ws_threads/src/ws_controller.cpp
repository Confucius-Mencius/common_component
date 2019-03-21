#include "ws_controller.h"
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "app_frame_conf_mgr_interface.h"
#include "http_callback.h"
#include "ws_callback.h"

namespace ws
{
static struct lws_protocols protocols[] =
{
    {
        "protocol-http",
        http::Callback,
        0,
        0,
        0, NULL, 0
    },
    {
        "protocol-ws",
        Callback,
        0, // 指定了回调时user指向的内存块的大小，这个内存由lws库管理
        0, // max frame size(recv buffer size)，超过这个大小的消息会分帧发送和接收，所以需要应用自己组包。设置为0是什么含义？不分帧吗？ TODO
        0, NULL, 0
    },
    { NULL, NULL, 0, 0, 0, NULL, 0 } /* 结束标志 */
};

static const struct lws_extension exts[] =
{
    {
        "permessage-deflate",
        lws_extension_callback_pm_deflate,
        "permessage-deflate; client_no_context_takeover; client_max_window_bits"
    },
    {
        "deflate-frame",
        lws_extension_callback_pm_deflate,
        "deflate_frame"
    },
    { NULL, NULL, NULL } /* 结束标志 */
};

Controller::Controller()
{
    threads_ctx_ = NULL;
    ws_thread_group_ = NULL;
    iface_[0] = '\0';
    iface_ip_[0] = '\0';
    cert_file_path_[0] = '\0';
    private_key_file_path_[0] = '\0';
    foreign_loops_ = NULL;
    ws_ctx_ = NULL;
    wss_ctx_ = NULL;
}

Controller::~Controller()
{
}

int Controller::Initialize(const ThreadsCtx* threads_ctx)
{
    threads_ctx_ = threads_ctx;

    // ws/wss both
    if (threads_ctx_->conf_mgr->GetWSIface().length() > 0)
    {
        strcpy(iface_, threads_ctx_->conf_mgr->GetWSIface().c_str());

        struct ifaddrs* ifaddr = NULL;

        if (getifaddrs(&ifaddr) != 0)
        {
            const int err = errno;
            LOG_ERROR("getifaddrs failed, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }

//        char netmask[INET_ADDRSTRLEN] = "";

        for (struct ifaddrs* ifp = ifaddr; ifp != NULL; ifp = ifp->ifa_next)
        {
            if (ifp->ifa_addr && ifp->ifa_addr->sa_family == AF_INET)
            {
                if (0 == strcmp(ifp->ifa_name, iface_))
                {
                    strncpy(iface_ip_, inet_ntoa(((struct sockaddr_in*)ifp->ifa_addr)->sin_addr), sizeof(iface_ip_));
//                    strncpy(netmask, inet_ntoa(((struct sockaddr_in*)ifp->ifa_netmask)->sin_addr), sizeof(netmask));
                    break;
                }
            }
        }

        freeifaddrs(ifaddr);

        if ('\0' == iface_ip_[0])
        {
            LOG_ERROR("iface not found: " << iface_);
            return -1;
        }
    }

    foreign_loops_ = new void* [ws_thread_group_->GetThreadCount()];
    if (NULL == foreign_loops_)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc memory, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    for (int i = 0; i < ws_thread_group_->GetThreadCount(); ++i)
    {
        foreign_loops_[i] = ws_thread_group_->GetThread(i)->GetThreadEvBase();
    }

    // ws
    if (CreateWSContext(false) != 0)
    {
        return -1;
    }

    // wss
    if (CreateWSContext(true) != 0)
    {
        return -1;
    }

    return 0;
}

void Controller::Finalize()
{
    if (wss_ctx_ != NULL)
    {
        lws_context_destroy(wss_ctx_);
        wss_ctx_ = NULL;
    }

    if (ws_ctx_ != NULL)
    {
        lws_context_destroy(ws_ctx_);
        ws_ctx_ = NULL;
    }

    if (foreign_loops_ != NULL)
    {
        delete [] foreign_loops_;
        foreign_loops_ = NULL;
    }
}

int Controller::CreateWSContext(bool use_ssl)
{
    unsigned int opts = LWS_SERVER_OPTION_ALLOW_LISTEN_SHARE | LWS_SERVER_OPTION_LIBEVENT;
    std::string ws = "ws";
    struct lws_context_creation_info* info = &ws_info_;

    if (use_ssl)
    {
        ws = "wss";
        info = &wss_info_;
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

    if ('\0' == iface_[0])
    {
        info->iface = NULL; // 监听所有ip
        LOG_ALWAYS(ws << " listen addr port: 0.0.0.0:" << info->port);
    }
    else
    {
        info->iface = iface_;
        LOG_ALWAYS(ws << " listen addr port: " << iface_ip_ << ":" << info->port);
    }

//    static const struct lws_http_mount mount =
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

    struct lws_protocol_vhost_options content_type_header =
    {
        NULL, NULL, "Content-Type", "application/json; charset=UTF-8"
    };

    struct lws_protocol_vhost_options cache_control_header =
    {
        &content_type_header, NULL, "Cache-Control", "no-cache"
    };

    struct lws_protocol_vhost_options pragma_header =
    {
        &cache_control_header, NULL, "Pragma", "no-cache"
    };

    info->protocols = protocols;
    info->extensions = exts;

    if (use_ssl)
    {
        strcpy(cert_file_path_, threads_ctx_->conf_mgr->GetWSSCertificateChainFilePath().c_str());
        LOG_ALWAYS("cert file: " << cert_file_path_);

        strcpy(private_key_file_path_, threads_ctx_->conf_mgr->GetWSSPrivateKeyFilePath().c_str());
        LOG_ALWAYS("private key file: " << private_key_file_path_);

        info->ssl_cert_filepath = cert_file_path_;
        info->ssl_private_key_filepath = private_key_file_path_;

        opts |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT; // TODO app frame中的ssl init看还要不要
        opts |= LWS_SERVER_OPTION_ALLOW_NON_SSL_ON_SSL_PORT;
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
    info->user = ws_thread_group_;
    info->options = opts;
    info->max_http_header_data = 0;
    info->max_http_header_data2 = LWS_RECOMMENDED_MIN_HEADER_SPACE; // The max amount of header payload that can be handled in an http request TODO 配置
    info->headers = &pragma_header;
    info->pt_serv_buf_size = 16384; // 0 = default of 4096. 如果为4096，超过会分帧。设置为16k提高吞吐
    info->ws_ping_pong_interval = 0; // 0 for none, else interval in seconds between sending PINGs on idle websocket connections.
    info->max_http_header_pool = 0; // The max number of connections with http headers that can be processed simultaneously. 0 = allow as many ah as number of availble fds for the process
    info->count_threads = ws_thread_group_->GetThreadCount(); // how many contexts to create in an array, 0 = 1
    info->fd_limit_per_thread = 0; // nonzero means restrict each service thread to this many fds, 0 means the default which is divide the process fd limit by the number of threads.
    info->timeout_secs = 0; // If nonzero, this member lets you set the timeout used in seconds. Otherwise a default timeout is used.
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
    info->foreign_loops = foreign_loops_;

    if (use_ssl)
    {
        wss_ctx_ = lws_create_context(info);
        if (NULL == wss_ctx_)
        {
            const int err = errno;
            LOG_ERROR("lws_create_context failed, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }

        LOG_DEBUG("wss threads count: " << lws_get_count_threads(wss_ctx_));
    }
    else
    {
        ws_ctx_ = lws_create_context(info);
        if (NULL == ws_ctx_)
        {
            const int err = errno;
            LOG_ERROR("lws_create_context failed, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }

        LOG_DEBUG("ws threads count: " << lws_get_count_threads(ws_ctx_));
    }

    return 0;
}
}
