#include "thread_sink.h"
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "str_util.h"
#include "task_type.h"

namespace ws
{
static void log_emit_function(int level, const char* msg)
{
    switch (level)
    {
        case LLL_ERR:
        {
            LOG_ERROR(msg);
        }
        break;

        case LLL_WARN:
        {
            LOG_WARN(msg);
        }
        break;

        case LLL_NOTICE:
        {
            LOG_INFO(msg);
        }
        break;

        default:
        {
        }
        break;
    }
}

static int callback_ws(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
    (void) wsi;
    (void) reason;
    (void) user;
    (void) in;
    (void) len;
//    std::cout << "wsi: " << wsi << ", reason: " << reason << ", user: " << user << ", in: " << in << ", len: " << len
//              << std::endl;

//    struct per_session_data__minimal* pss =
//        (struct per_session_data__minimal*) user;

//    /*由vhost与protocol还原lws_protocol_vh_priv_zalloc申请的结构*/
//    struct per_vhost_data__minimal* vhd =
//        (struct per_vhost_data__minimal*)
//        lws_protocol_vh_priv_get(lws_get_vhost(wsi),
//                                 lws_get_protocol(wsi));

//    const struct lws_protocols* protocol = lws_get_protocol(wsi);
//    if (protocol != NULL)
//    {
//        std::cout << protocol->name << ", " << protocol->per_session_data_size
//                  << ", " << protocol->rx_buffer_size << ", " << protocol->id << ", "
//                  << protocol->tx_packet_size << std::endl;
//    }

//    int m;

//    switch (reason)
//    {
//        case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
//            std::cout << "LWS_CALLBACK_FILTER_NETWORK_CONNECTION" << std::endl;
//            break;

//        case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
//            std::cout << "LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED" << std::endl;
//            break;

//        case LWS_CALLBACK_WSI_CREATE:
//            std::cout << "LWS_CALLBACK_WSI_CREATE" << std::endl;
//            break;

//        case LWS_CALLBACK_ADD_POLL_FD:
//            std::cout << "LWS_CALLBACK_ADD_POLL_FD" << std::endl;
//            break;

//        case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
//            std::cout << "LWS_CALLBACK_CHANGE_MODE_POLL_FD" << std::endl;
//            break;

//        case LWS_CALLBACK_LOCK_POLL:
//            std::cout << "LWS_CALLBACK_LOCK_POLL" << std::endl;
//            break;

//        case LWS_CALLBACK_UNLOCK_POLL:
//            std::cout << "LWS_CALLBACK_UNLOCK_POLL" << std::endl;
//            break;

//        case LWS_CALLBACK_DEL_POLL_FD:
//            std::cout << "LWS_CALLBACK_DEL_POLL_FD" << std::endl;
//            break;

//        case LWS_CALLBACK_WSI_DESTROY:
//            std::cout << "LWS_CALLBACK_WSI_DESTROY" << std::endl;
//            break;

//        case LWS_CALLBACK_PROTOCOL_DESTROY:
//            std::cout << "LWS_CALLBACK_PROTOCOL_DESTROY" << std::endl;
//            break;

//        case LWS_CALLBACK_GET_THREAD_ID:
//            std::cout << "LWS_CALLBACK_GET_THREAD_ID" << std::endl;
//            break;

//        /*初始化*/
//        // wsi=0x7fffffffd3f0, reason=LWS_CALLBACK_PROTOCOL_INIT, user=0x0, in=0x0, len=0
//        // 服务启动的时候就会初始化，初始化的时候，上面的pss和vhd都为null。
//        case LWS_CALLBACK_PROTOCOL_INIT:
//            std::cout << "LWS_CALLBACK_PROTOCOL_INIT" << std::endl;

//            /*申请内存*/
//            vhd = (struct per_vhost_data__minimal*) lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi),
//                    lws_get_protocol(wsi),
//                    sizeof(struct per_vhost_data__minimal));
//            vhd->context = lws_get_context(wsi);
//            vhd->protocol = lws_get_protocol(wsi);
//            vhd->vhost = lws_get_vhost(wsi);

//            // 初始化完毕的结果：
//            //            (gdb) p vhd
//            //            $2 = (per_vhost_data__minimal *) 0x615bb0
//            //            (gdb) p *vhd
//            //            $3 = {context = 0x614ca0, vhost = 0x615260, protocol = 0x602158 <protocols+56>, pss_list = 0x0, amsg = {payload = 0x0, len = 0}, current = 0}
//            //            (gdb) p vhd->protocol
//            //            $4 = (const lws_protocols *) 0x602158 <protocols+56>
//            //            (gdb) p *vhd->protocol
//            //            $5 = {name = 0x40194d "lws-minimal", callback = 0x4011c3 <callback_minimal(lws*, lws_callback_reasons, void*, void*, size_t)>, per_session_data_size = 24, rx_buffer_size = 128, id = 0, user = 0x0, tx_packet_size = 0}

//            break;

//        case LWS_CALLBACK_EVENT_WAIT_CANCELLED: // 不用处理
//            // 这里的wsi与init的不一样。所有的LWS_CALLBACK_EVENT_WAIT_CANCELLED事件的wsi都是一个
//            std::cout << "LWS_CALLBACK_EVENT_WAIT_CANCELLED" << std::endl;
//            break;

//        // 在LWS_CALLBACK_ESTABLISHED之前，依次会收到下面2个事件，可以不用处理。这里打出来理解流程
//        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
//            /*
//             * this just demonstrates how to use the protocol filter. If you won't
//             * study and reject connections based on header content, you don't need
//             * to handle this callback
//             */
//            std::cout << "LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION" << std::endl;
//            dump_handshake_info(wsi);
//            /* you could return non-zero here and kill the connection */
//            break;
//        case LWS_CALLBACK_ADD_HEADERS:
//            std::cout << "LWS_CALLBACK_ADD_HEADERS" << std::endl;

//            break;

//        /*建立连接，将客户端放入客户端链表*/
//        // 当有新连接来时，会收到几条“LWS_CALLBACK_EVENT_WAIT_CANCELLED，这个事件是wsi都一样”，
//        // 然后是LWS_CALLBACK_ESTABLISHED，接着是LWS_CALLBACK_SERVER_WRITEABLE，这两个事件的wsi是一样的，标示了这个客户端连接
//        case LWS_CALLBACK_ESTABLISHED:
//            std::cout << "LWS_CALLBACK_ESTABLISHED" << std::endl;

//            /* add ourselves to the list of live pss held in the vhd */
//            lws_ll_fwd_insert(pss, pss_list, vhd->pss_list);
//            pss->wsi = wsi;
//            pss->last = vhd->current;
//            break;

//        case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
//            // 在LWS_CALLBACK_CLOSED之前会有这个事件，收到的数据长度为2(0xE903)
//            /*
//             * this just demonstrates how to handle
//             * LWS_CALLBACK_WS_PEER_INITIATED_CLOSE and extract the peer's close
//             * code and auxiliary data.  You can just not handle it if you don't
//             * have a use for this.
//             */
//            std::cout << "LWS_CALLBACK_WS_PEER_INITIATED_CLOSE" << std::endl;

//            for (int n = 0; n < (int) len; n++)
//            {
//                lwsl_notice(" %d: 0x%02X\n", n,
//                            ((unsigned char*) in)[n]);
//            }
//            break;

//        /*连接关闭，将客户端从链表中移除*/
//        // 关闭客户端时，会收到LWS_CALLBACK_CLOSED事件，wsi标示了这个客户端连接。但是连接断开时不一定会触发这个事件？？
//        case LWS_CALLBACK_CLOSED:
//            std::cout << "LWS_CALLBACK_CLOSED" << std::endl;

//            /* remove our closing pss from the list of live pss */
//            lws_ll_fwd_remove(struct per_session_data__minimal, pss_list,
//                              pss, vhd->pss_list);
//            break;

//        case LWS_CALLBACK_SERVER_WRITEABLE:
//            //        表示wsi对应的ws连接当前处于可写状态，即：可发送数据至客户端。
//            std::cout << "LWS_CALLBACK_SERVER_WRITEABLE" << std::endl;
//            std::cout << "LWS_PRE: " << LWS_PRE << std::endl;

//            if (!vhd->amsg.payload)
//            {
//                std::cout << "payload is null" << std::endl;
//                break;
//            }

//            std::cout << "last msg num: " << pss->last << ", current: " << vhd->current << std::endl;

//            if (pss->last == vhd->current)
//            {
//                break;
//            }

//            /* notice we allowed for LWS_PRE in the payload already */
//            m = lws_write(wsi, (unsigned char*) vhd->amsg.payload + LWS_PRE, vhd->amsg.len,
//                          LWS_WRITE_TEXT);
//            if (m < (int) vhd->amsg.len)
//            {
//                lwsl_err("ERROR %d writing to ws\n", m);
//                return -1;
//            }

//            pss->last = vhd->current;
//            break;

//        // 客户端发送数据时，会触发LWS_CALLBACK_RECEIVE，wsi与连接时一样
//        //            表示ＷＳ服务端收到客户端发送过来的一帧完整数据，此时表1中的in表示收到的数据，len表示收到的数据长度。
//        //            需要注意的是：指针in的回收、释放始终由LWS框架管理，只要出了回调函数，该空间就会被LWS框架回收。因此，开发者若想将接收的数据进行转发，则必须对该数据进行拷贝。
//        case LWS_CALLBACK_RECEIVE:
//            std::cout << "LWS_CALLBACK_RECEIVE" << std::endl;
//            std::cout << "LWS_PRE: " << LWS_PRE << ", lws_is_final_fragment(wsi): " << lws_is_final_fragment(wsi) << std::endl;
//            lwsl_user("RX: %s\n", (const char*) in);

//            if (vhd->amsg.payload)
//            {
//                __minimal_destroy_message(&vhd->amsg);
//            }

//            vhd->amsg.len = len;
//            /* notice we over-allocate by LWS_PRE */
//            vhd->amsg.payload = (unsigned char*) malloc(LWS_PRE + len + 1);
//            if (!vhd->amsg.payload)
//            {
//                lwsl_user("OOM: dropping\n");
//                break;
//            }

//            memcpy((char*) vhd->amsg.payload + LWS_PRE, in, len); // TODO LWS_PRE是干嘛的？
//            vhd->amsg.payload[LWS_PRE + len] = '\0';
//            vhd->current++;
//            std::cout << "current msg num: " << vhd->current << std::endl;

//            /*
//             * let everybody know we want to write something on them
//             * as soon as they are ready
//             */
//            /*遍历所有的客户端，将数据放入写入回调*/
//            lws_start_foreach_llp(struct per_session_data__minimal**,
//                                  ppss, vhd->pss_list)
//            {
//                lws_callback_on_writable((*ppss)->wsi); // 这个调用会触发LWS_CALLBACK_SERVER_WRITEABLE，wsi时这个客户端连接的wsi
//            }
//            lws_end_foreach_llp(ppss, pss_list);
//            break;

//        default:
//            break;
//    }

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
        10, // max frame size(recv buffer size)，超过这个大小的消息会分帧发送和接收，所以需要应用自己组包。设置为0是什么含义？不分帧吗？
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
    {NULL, NULL, NULL /* terminator */ }
};

ThreadSink::ThreadSink()
    : common_logic_loader_(), logic_item_vec_(), conn_mgr_(), scheduler_()
{
    threads_ctx_ = NULL;
    ws_thread_group_ = NULL;
    related_thread_group_ = NULL;
    common_logic_ = NULL;
}

ThreadSink::~ThreadSink()
{
}

void ThreadSink::Release()
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_RELEASE_MODULE(it->logic, it->logic_loader);
    }

    logic_item_vec_.clear();
    SAFE_RELEASE_MODULE(common_logic_, common_logic_loader_);
    conn_mgr_.Release();

    delete this;
}

int ThreadSink::OnInitialize(ThreadInterface* thread, const void* ctx)
{
    if (ThreadSinkInterface::OnInitialize(thread, ctx) != 0)
    {
        return -1;
    }

    threads_ctx_ = static_cast<const ThreadsCtx*>(ctx);

    lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE, log_emit_function);

    if (BindWSSocket() != 0)
    {
        return -1;
    }

    conn_mgr_.SetThreadSink(this);

    ConnMgrCtx conn_mgr_ctx;
    conn_mgr_ctx.timer_axis = self_thread_->GetTimerAxis();

    conn_mgr_ctx.inactive_conn_check_interval =
    {
        threads_ctx_->conf_mgr->GetWSInactiveConnCheckIntervalSec(),
        threads_ctx_->conf_mgr->GetWSInactiveConnCheckIntervalUsec()
    };

    conn_mgr_ctx.inactive_conn_life = threads_ctx_->conf_mgr->GetWSInactiveConnLife();
    conn_mgr_ctx.storm_interval = threads_ctx_->conf_mgr->GetWSStormInterval();
    conn_mgr_ctx.storm_threshold = threads_ctx_->conf_mgr->GetWSStormThreshold();

    if (conn_mgr_.Initialize(&conn_mgr_ctx) != 0)
    {
        return -1;
    }

    scheduler_.SetThreadSink(this);

    if (scheduler_.Initialize(threads_ctx_) != 0)
    {
        return -1;
    }

    if (LoadCommonLogic() != 0)
    {
        return -1;
    }

    if (LoadLogicGroup() != 0)
    {
        return -1;
    }

    return 0;
}

void ThreadSink::OnFinalize()
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_FINALIZE(it->logic);
    }

    SAFE_FINALIZE(common_logic_);
    scheduler_.Finalize();
    conn_mgr_.Finalize();

    lws_context_destroy(ws_context_);

    ThreadSinkInterface::OnFinalize();
}

int ThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    if (conn_mgr_.Activate() != 0)
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(common_logic_))
    {
        return -1;
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        if (SAFE_ACTIVATE_FAILED(it->logic))
        {
            return -1;
        }
    }

    return 0;
}

void ThreadSink::OnFreeze()
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_FREEZE(it->logic);
    }

    SAFE_FREEZE(common_logic_);
    conn_mgr_.Freeze();
    ThreadSinkInterface::OnFreeze();
}

void ThreadSink::OnThreadStartOK()
{
    ThreadSinkInterface::OnThreadStartOK();

    pthread_mutex_lock(threads_ctx_->app_frame_threads_sync_mutex);
    ++(*threads_ctx_->app_frame_threads_count);
    pthread_cond_signal(threads_ctx_->app_frame_threads_sync_cond);
    pthread_mutex_unlock(threads_ctx_->app_frame_threads_sync_mutex);
}

void ThreadSink::OnStop()
{
    ThreadSinkInterface::OnStop();

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnStop();
    }

    if (common_logic_ != NULL)
    {
        common_logic_->OnStop();
    }
}

void ThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnReload();
    }

    if (common_logic_ != NULL)
    {
        common_logic_->OnReload();
    }
}

void ThreadSink::OnTask(const ThreadTask* task)
{
    ThreadSinkInterface::OnTask(task);

    switch (task->GetType())
    {
        case TASK_TYPE_TCP_SEND_TO_CLIENT:
        {
            scheduler_.SendToClient(task->GetConnGUID(), task->GetData().data(), task->GetData().size());
        }
        break;

//        case TASK_TYPE_WS_CLOSE_CONN:
//        {
//            scheduler_.CloseClient(task->GetConnGUID());
//        }
//        break;

        case TASK_TYPE_NORMAL:
        {
            for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
            {
                it->logic->OnTask(task->GetConnGUID(), task->GetSourceThread(),
                                  task->GetData().data(), task->GetData().size());
            }
        }
        break;

        default:
        {
            LOG_ERROR("invalid task type: " << task->GetType());
        }
        break;
    }
}

bool ThreadSink::CanExit() const
{
    int can_exit = 1;

    for (LogicItemVec::const_iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        can_exit &= (it->logic->CanExit() ? 1 : 0);
    }

    if (common_logic_ != NULL)
    {
        can_exit &= (common_logic_->CanExit() ? 1 : 0);
    }

    return (can_exit != 0);
}

void ThreadSink::OnClientClosed(const BaseConn* conn)
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientClosed(conn->GetConnGUID());
    }

    if (common_logic_ != NULL)
    {
        common_logic_->OnClientClosed(conn->GetConnGUID());
    }

    char client_ctx_buf[128] = "";
    const int n = StrPrintf(client_ctx_buf, sizeof(client_ctx_buf), "%s:%u, socket fd: %d",
                            conn->GetClientIP(), conn->GetClientPort(), conn->GetSockFD());

    ThreadTask* task = new ThreadTask(TASK_TYPE_TCP_CONN_CLOSED, self_thread_, NULL, client_ctx_buf, n);
    if (NULL == task)
    {
        LOG_ERROR("failed to create tcp conn closed task");
        return;
    }

    listen_thread_->PushTask(task);
    conn_mgr_.DestroyConn(conn->GetSockFD());
}

void ThreadSink::SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups)
{
    related_thread_group_ = related_thread_groups;

    if (related_thread_group_->global_logic != NULL)
    {
        if (common_logic_ != NULL)
        {
            common_logic_->SetGlobalLogic(related_thread_group_->global_logic);
        }

        for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
        {
            LogicItem& logic_item = *it;
            logic_item.logic->SetGlobalLogic(related_thread_group_->global_logic);
        }
    }

    scheduler_.SetRelatedThreadGroups(related_thread_groups);
}

int ThreadSink::BindWSSocket()
{
    memset(&ws_info_, 0, sizeof ws_info_);
    ws_info_.port = threads_ctx_->conf_mgr->GetWSPort();

    if (0 == threads_ctx_->conf_mgr->GetWSIface().length())
    {
        ws_info_.iface = NULL; // 监听所有ip
        LOG_ALWAYS("ws listen addr port: 0.0.0.0:" << threads_ctx_->conf_mgr->GetWSPort());
    }
    else
    {
        strcpy(ws_iface_, threads_ctx_->conf_mgr->GetWSIface().c_str());
        ws_info_.iface = ws_iface_;

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
                if (strcmp(ifp->ifa_name, ws_iface_))
                {
                    strncpy(ip, inet_ntoa(((struct sockaddr_in*)ifp->ifa_addr)->sin_addr), 16);
                    strncpy(netmask, inet_ntoa(((struct sockaddr_in*)ifp->ifa_netmask)->sin_addr), 16);
                    break;
                }
            }
        }

        freeifaddrs(ifaddr);
        LOG_ALWAYS("ws listen addr port: " << ip << ":" << threads_ctx_->conf_mgr->GetWSPort());
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

    ws_info_.mounts = NULL; // &http_mount_;

    ws_info_.protocols = protocols;
    ws_info_.extensions = exts;

    ws_info_.ssl_cert_filepath = NULL;
    ws_info_.ssl_private_key_filepath = NULL;

//    bool use_ssl = false;
//    if (use_ssl)
//    {
////        sprintf(cert_path, "%s/libwebsockets-test-server.pem",
////                resource_path);
////        sprintf(key_path, "%s/libwebsockets-test-server.key.pem",
////                resource_path);

//        info.ssl_cert_filepath = cert_path;
//        info.ssl_private_key_filepath = key_path;
//        opts |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
//    }

    ws_info_.gid = -1;
    ws_info_.uid = -1;
    ws_info_.max_http_header_data = 1024;
    ws_info_.max_http_header_pool = 0;
    ws_info_.options = LWS_SERVER_OPTION_LIBEVENT;

    ws_foreign_loops_[0] = self_thread_->GetThreadEvBase();
    ws_info_.foreign_loops = ws_foreign_loops_;

    ws_context_ = lws_create_context(&ws_info_);
    if (NULL == ws_context_)
    {
        LOG_ERROR("libwebsocket init failed");
        return -1;
    }

    return 0;
}

int ThreadSink::LoadCommonLogic()
{
    const std::string ws_common_logic_so = threads_ctx_->conf_mgr->GetWSCommonLogicSo();
    if (0 == ws_common_logic_so.length())
    {
        return 0;
    }

    char common_logic_so_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(common_logic_so_path, sizeof(common_logic_so_path),
                    ws_common_logic_so.c_str(), threads_ctx_->cur_working_dir);
    LOG_TRACE("load common logic so " << common_logic_so_path << " begin");

    if (common_logic_loader_.Load(common_logic_so_path) != 0)
    {
        LOG_ERROR("failed to load common logic so " << common_logic_so_path
                  << ", " << common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    common_logic_ = static_cast<CommonLogicInterface*>(common_logic_loader_.GetModuleInterface());
    if (NULL == common_logic_)
    {
        LOG_ERROR("failed to get common logic, " << common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    LogicCtx logic_ctx;
    logic_ctx.argc = threads_ctx_->argc;
    logic_ctx.argv = threads_ctx_->argv;
    logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
    logic_ctx.cur_working_dir = threads_ctx_->cur_working_dir;
    logic_ctx.app_name = threads_ctx_->app_name;
    logic_ctx.conf_center = threads_ctx_->conf_center;
    logic_ctx.timer_axis = self_thread_->GetTimerAxis();
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.common_logic = common_logic_;
    logic_ctx.thread_ev_base = self_thread_->GetThreadEvBase();

    if (common_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_TRACE("load common logic so " << common_logic_so_path << " end");
    return 0;
}

int ThreadSink::LoadLogicGroup()
{
    // logic so group
    LogicItem logic_item;
    logic_item.logic = NULL;

    const StrGroup logic_so_group = threads_ctx_->conf_mgr->GetWSLogicSoGroup();

    for (StrGroup::const_iterator it = logic_so_group.begin();
            it != logic_so_group.end(); ++it)
    {
        char logic_so_path[MAX_PATH_LEN] = "";
        GetAbsolutePath(logic_so_path, sizeof(logic_so_path), (*it).c_str(), threads_ctx_->cur_working_dir);
        logic_item.logic_so_path = logic_so_path;
        logic_item_vec_.push_back(logic_item);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        LogicItem& logic_item = *it;
        LOG_TRACE("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so " << logic_item.logic_so_path << ", "
                      << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = static_cast<LogicInterface*>(logic_item.logic_loader.GetModuleInterface());
        if (NULL == logic_item.logic)
        {
            LOG_ERROR("failed to get logic, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        LogicCtx logic_ctx;
        logic_ctx.argc = threads_ctx_->argc;
        logic_ctx.argv = threads_ctx_->argv;
        logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
        logic_ctx.cur_working_dir = threads_ctx_->cur_working_dir;
        logic_ctx.app_name = threads_ctx_->app_name;
        logic_ctx.conf_center = threads_ctx_->conf_center;
        logic_ctx.timer_axis = self_thread_->GetTimerAxis();
        logic_ctx.scheduler = &scheduler_;
        logic_ctx.common_logic = common_logic_;
        logic_ctx.thread_ev_base = self_thread_->GetThreadEvBase();

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_TRACE("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}

//void ThreadSink::OnClientConnected(const NewConnCtx* new_conn_ctx)
//{
//    BaseConn* conn = conn_mgr_.GetConn(new_conn_ctx->client_sock_fd);
//    if (conn != NULL)
//    {
//        LOG_WARN("ws conn already exist, socket fd: " << new_conn_ctx->client_sock_fd << ", destroy it first");
//        conn_mgr_.DestroyConn(conn->GetSockFD());
//    }

//    conn = conn_mgr_.CreateConn(self_thread_->GetThreadIdx(), new_conn_ctx->client_ip,
//                                new_conn_ctx->client_port, new_conn_ctx->client_sock_fd);
//    if (NULL == conn)
//    {
//        char client_ctx_buf[128] = "";
//        const int n = StrPrintf(client_ctx_buf, sizeof(client_ctx_buf), "%s:%u, socket fd: %d",
//                                new_conn_ctx->client_ip, new_conn_ctx->client_port,
//                                new_conn_ctx->client_sock_fd);

//        ThreadTask* task = new ThreadTask(TASK_TYPE_TCP_CONN_CLOSED, self_thread_, NULL, client_ctx_buf, n);
//        if (NULL == task)
//        {
//            LOG_ERROR("failed to create ws conn closed task");
//            return;
//        }

//        listen_thread_->PushTask(task);
//        return;
//    }

//    if (common_logic_ != NULL)
//    {
//        common_logic_->OnClientConnected(conn->GetConnGUID());
//    }

//    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
//    {
//        (*it).logic->OnClientConnected(conn->GetConnGUID());
//    }
//}

void ThreadSink::OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len)
{
    if (common_logic_ != NULL)
    {
        common_logic_->OnRecvClientData(conn_guid, data, len);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnRecvClientData(conn_guid, data, len);
    }
}
}
