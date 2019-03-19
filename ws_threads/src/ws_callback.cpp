#include "ws_callback.h"
#include "conn.h"
#include "log_util.h"
#include "thread_center_interface.h"
#include "thread_sink.h"
#include "ws_util.h"

namespace ws
{
int WSCallback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len)
{
    (void) user;

//    LOG_TRACE("wsi: " << wsi << ", reason: " << reason << ", user: " << user << ", in: " << in << ", len: " << len);

//    struct per_session_data* pss =
//        (struct per_session_data*) user;

    // 由vhost与protocol获取通过lws_protocol_vh_priv_zalloc分配的结构
    struct per_vhost_data* vhd = (struct per_vhost_data*) lws_protocol_vh_priv_get(lws_get_vhost(wsi), lws_get_protocol(wsi));

//    const struct lws_protocols* protocol = lws_get_protocol(wsi);
//    if (protocol != NULL)
//    {
//        LOG_DEBUG(protocol->name << ", " << protocol->per_session_data_size
//                  << ", " << protocol->rx_buffer_size << ", " << protocol->id << ", "
//                  << protocol->tx_packet_size);
//    }

    switch (reason)
    {
#if 0
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
#endif
        case LWS_CALLBACK_PROTOCOL_DESTROY:
        {
            LOG_TRACE("LWS_CALLBACK_PROTOCOL_DESTROY");

            // 销毁自己分配的一些资源
        }
        break;
#if 0
        case LWS_CALLBACK_GET_THREAD_ID:
            LOG_TRACE("LWS_CALLBACK_GET_THREAD_ID");
            break;
#endif
        // 初始化
        case LWS_CALLBACK_PROTOCOL_INIT:
        {
            LOG_TRACE("LWS_CALLBACK_PROTOCOL_INIT");

            // 分配vhost+protocol相关的存储块
            vhd = (struct per_vhost_data*) lws_protocol_vh_priv_zalloc(lws_get_vhost(wsi), lws_get_protocol(wsi), sizeof(struct per_vhost_data));
            if (NULL == vhd)
            {
                const int err = errno;
                LOG_ERROR("lws_protocol_vh_priv_zalloc failed, errno: " << err << ", err msg: " << strerror(err));
                return -1;
            }

            vhd->context = lws_get_context(wsi);
            vhd->vhost = lws_get_vhost(wsi);
            vhd->protocol = lws_get_protocol(wsi);


            // 自己分配一些资源
        }
        break;
#if 0
        case LWS_CALLBACK_EVENT_WAIT_CANCELLED: // 不用处理
            // 这里的wsi与init时的不一样。所有的LWS_CALLBACK_EVENT_WAIT_CANCELLED事件的wsi都是一个，是全局的
            LOG_TRACE("LWS_CALLBACK_EVENT_WAIT_CANCELLED");
            break;
#endif
        // 在LWS_CALLBACK_ESTABLISHED之前，依次会收到下面2个事件，可以不用处理。这里打出来理解流程
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        {
            /*
             * this just demonstrates how to use the protocol filter. If you won't
             * study and reject connections based on header content, you don't need
             * to handle this callback
             */
            LOG_TRACE("LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION");
            DumpRequestHeaders(wsi);
            /* you could return non-zero here and kill the connection */
        }
        break;
#if 0
        case LWS_CALLBACK_ADD_HEADERS:
        {
            // TODO 添加额外的http头
            LOG_TRACE("LWS_CALLBACK_ADD_HEADERS");
        }
        break;
#endif
        // 连接建立成功
        // 当有新连接来时，会收到几条LWS_CALLBACK_EVENT_WAIT_CANCELLED，这个事件是wsi都一样，
        // 然后是LWS_CALLBACK_ESTABLISHED，接着是LWS_CALLBACK_SERVER_WRITEABLE，后面两个事件的wsi是一样的，标示了这个客户端连接
        case LWS_CALLBACK_ESTABLISHED:
        {
            LOG_TRACE("LWS_CALLBACK_ESTABLISHED");
            ThreadGroupInterface* ws_thread_group = static_cast<ThreadGroupInterface*>(lws_vhost_user(lws_get_vhost(wsi)));
            ThreadSink* thread_sink = static_cast<ThreadSink*>(ws_thread_group->GetThread(0)->GetThreadSink()); // TODO

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

            conn->SetWSI(wsi);
        }
        break;
#if 0
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
#endif
        // 连接关闭
        // 关闭客户端时，会收到LWS_CALLBACK_CLOSED事件，wsi标示了这个客户端连接
        case LWS_CALLBACK_CLOSED:
        {
            LOG_TRACE("LWS_CALLBACK_CLOSED");
            ThreadGroupInterface* ws_thread_group = static_cast<ThreadGroupInterface*>(lws_vhost_user(lws_get_vhost(wsi)));
            ThreadSink* thread_sink = static_cast<ThreadSink*>(ws_thread_group->GetThread(0)->GetThreadSink()); // TODO

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
            // 表示wsi对应的ws连接当前处于可写状态，可发送数据至客户端。
            LOG_TRACE("LWS_CALLBACK_SERVER_WRITEABLE");

            ThreadGroupInterface* ws_thread_group = static_cast<ThreadGroupInterface*>(lws_vhost_user(lws_get_vhost(wsi)));
            ThreadSink* thread_sink = static_cast<ThreadSink*>(ws_thread_group->GetThread(0)->GetThreadSink()); // TODO

            const lws_sockfd_type sock_fd = lws_get_socket_fd(wsi);

            ConnMgr* conn_mgr = thread_sink->GetConnMgr();
            Conn* conn = static_cast<Conn*>(conn_mgr->GetConn(sock_fd));
            if (NULL == conn)
            {
                LOG_ERROR("failed to get ws conn by socket fd: " << sock_fd);
                return -1;
            }

            conn->OnWrite();
        }
        break;

        // 收到客户端的数据
        // 客户端发送数据时，会触发LWS_CALLBACK_RECEIVE，wsi与建立连接时一样
        // 服务端此时收到客户端发送过来的数据，参数in表示收到的数据，len表示数据长度
        // 需要注意的是：指针in的回收、释放始终由LWS框架管理，只要出了回调函数，该空间就会被LWS框架回收。因此，开发者若想将接收的数据进行转发，则必须对该数据进行拷贝。
        case LWS_CALLBACK_RECEIVE:
        {
            LOG_TRACE("LWS_CALLBACK_RECEIVE");
            LOG_DEBUG("in: " << (char*) in << ", len: " << len << ", is final fragment: " << lws_is_final_fragment(wsi));

            ThreadGroupInterface* ws_thread_group = static_cast<ThreadGroupInterface*>(lws_vhost_user(lws_get_vhost(wsi)));
            ThreadSink* thread_sink = static_cast<ThreadSink*>(ws_thread_group->GetThread(0)->GetThreadSink()); // TODO

            if (thread_sink->GetThread()->IsStopping())
            {
                LOG_WARN("in stopping status, refuse all client data");
                return -1;
            }

            // TODO 组帧，收到一个完整的帧才抛给应用层
            const lws_sockfd_type sock_fd = lws_get_socket_fd(wsi);

            ConnMgr* conn_mgr = thread_sink->GetConnMgr();
            Conn* conn = static_cast<Conn*>(conn_mgr->GetConn(sock_fd));
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

            std::string& data = conn->AppendData(in, len);

            if (lws_is_final_fragment(wsi))
            {
                thread_sink->OnRecvClientData(conn->GetConnGUID(), data.data(), data.size());
                conn->EraseData();
            }
        }
        break;

        default:
        {
        }
        break;
    }

    return 0;
}
}
