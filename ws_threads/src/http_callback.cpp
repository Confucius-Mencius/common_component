#include "http_callback.h"
#include "log_util.h"
#include "thread_center_interface.h"
#include "thread_sink.h"
#include "http_conn.h"
#include "ws_util.h"

namespace ws
{
namespace http
{
int Callback(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len)
{
    LOG_TRACE("wsi: " << wsi << ", reason: " << reason << ", user: " << user << ", in: " << in << ", len: " << len);

    uint8_t rsp_header_buf[LWS_PRE + LWS_RECOMMENDED_MIN_HEADER_SPACE] = "";
    uint8_t* start = &rsp_header_buf[LWS_PRE];
    uint8_t* p = start;
    uint8_t* end = &rsp_header_buf[sizeof(rsp_header_buf) - 1];

    switch (reason)
    {
#if 0
        case LWS_CALLBACK_FILTER_HTTP_CONNECTION: // 如果这里进去了，LWS_CALLBACK_HTTP里面的解析会乱掉
            LOG_TRACE("LWS_CALLBACK_FILTER_HTTP_CONNECTION");
            break;
#endif
        case LWS_CALLBACK_HTTP:
        {
            LOG_TRACE("LWS_CALLBACK_HTTP");

            if (len < 1)
            {
                lws_return_http_status(wsi, HTTP_STATUS_BAD_REQUEST, NULL);
                goto try_to_reuse;
            }

            DumpAllToken(wsi); // TODO 这个接口移到conn中去，解析出headers，放map中传给logic

            ThreadGroupInterface* thread_group = static_cast<ThreadGroupInterface*>(lws_vhost_user(lws_get_vhost(wsi)));
            const int thread_idx = *((int*) pthread_getspecific(thread_group->GetSpecificDataKey()));
            LOG_DEBUG("thread idx: " << thread_idx);
            ThreadSink* thread_sink = static_cast<ThreadSink*>(thread_group->GetThread(thread_idx)->GetThreadSink());

            if (thread_sink->GetThread()->IsStopping())
            {
                LOG_WARN("in stopping status, refuse all client data");
                return 1;
            }

            NewConnCtx new_conn_ctx;
            new_conn_ctx.client_sock_fd = lws_get_socket_fd(wsi);

            struct sockaddr_in client_addr;
            socklen_t addr_len = sizeof(client_addr);
            if (getpeername(new_conn_ctx.client_sock_fd, (struct sockaddr*) &client_addr, &addr_len) == -1)
            {
                const int err = errno;
                LOG_ERROR("getpeername failed, socket fd: " << new_conn_ctx.client_sock_fd << ", errno: " << err << ", err msg: " << strerror((err)));
                lws_return_http_status(wsi, HTTP_STATUS_INTERNAL_SERVER_ERROR, NULL);
                goto try_to_reuse;
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

            new_conn_ctx.conn_type = lws_is_ssl(wsi) ? CONN_TYPE_HTTPS : CONN_TYPE_HTTP;

            if (thread_sink->OnClientConnected(&new_conn_ctx) != 0)
            {
                lws_return_http_status(wsi, HTTP_STATUS_INTERNAL_SERVER_ERROR, NULL);
                goto try_to_reuse;
            }

            http::Conn* conn = static_cast<http::Conn*>(thread_sink->GetConnMgr()->GetConn(new_conn_ctx.client_sock_fd));
            if (NULL == conn)
            {
                LOG_ERROR("failed to get ws conn by socket fd: " << new_conn_ctx.client_sock_fd);
                lws_return_http_status(wsi, HTTP_STATUS_INTERNAL_SERVER_ERROR, NULL);
                goto try_to_reuse;
            }

            conn->SetWSI(wsi);

            // get or post?
            bool is_get = true;

            lws_token_indexes token_idx = WSI_TOKEN_GET_URI;
            int length = lws_hdr_total_length(wsi, token_idx);
            if (length > 0)
            {
                LOG_TRACE("is get");
            }
            else
            {
                is_get = false;

                token_idx = WSI_TOKEN_POST_URI;
                length = lws_hdr_total_length(wsi, token_idx);
                if (length > 0)
                {
                    LOG_TRACE("is post");
                }
                else
                {
                    LOG_ERROR("not support");
                    return 1;
                }
            }

            std::unique_ptr<char[]> buf(new char[length + 1]);
            if (nullptr == buf)
            {
                LOG_ERROR("failed to alloc memory");
                lws_return_http_status(wsi, HTTP_STATUS_INTERNAL_SERVER_ERROR, NULL);
                goto try_to_reuse;
            }

            lws_hdr_copy(wsi, buf.get(), length + 1, WSI_TOKEN_GET_URI);
            buf[length] = '\0';
            conn->SetPath(buf.get(), length);

//            const char* prot;
//            const char* ads;
//            int port;
//            const char* path;
//            int ret = lws_parse_uri((char*) in, &prot, &ads, &port, &path);
//            LOG_DEBUG(ret);
//            LOG_DEBUG("proto: " << prot << ", addr: " << ads << ", port: " << port << ", path: " << path);

//            lws_urlencode();
//            lws_urldecode();

            // TODO 解析出query的key和value，放map中传给logic
            for (int i = 0; ; ++i)
            {
                const int length = lws_hdr_fragment_length(wsi, WSI_TOKEN_HTTP_URI_ARGS, i);
                if (length <= 0)
                {
                    break;
                }

                std::unique_ptr<char[]> buf(new char[length + 1]);
                if (nullptr == buf)
                {
                    LOG_ERROR("failed to alloc memory");
                    continue;
                }

                lws_hdr_copy_fragment(wsi, buf.get(), length + 1, WSI_TOKEN_HTTP_URI_ARGS, i);
                buf[length] = '\0';
                LOG_DEBUG("uri arg " << i << ": " <<  buf.get()); // ?x=1&y=2
            }

//            lws_get_urlarg_by_name(wsi, "x=", buf, sizeof(buf));
//            LOG_DEBUG(buf << " " << buf[strlen("x=")]);

//            lws_get_urlarg_by_name(wsi, "y=", buf, sizeof(buf));
//            LOG_DEBUG(buf << " " << buf[strlen("y=")]);

            if (lws_add_http_header_status(wsi, HTTP_STATUS_OK, &p, end) != 0)
            {
                LOG_ERROR("failed to add status code");
                lws_return_http_status(wsi, HTTP_STATUS_INTERNAL_SERVER_ERROR, NULL);
                goto try_to_reuse;
            }

//            if (lws_add_http_header_by_token(wsi,
//                                             WSI_TOKEN_HTTP_CONTENT_TYPE,
//                                             (const unsigned char*) "application/json; charset=UTF-8",
//                                             strlen("application/json; charset=UTF-8"), &p, end) != 0)
//            {
//                LOG_ERROR("failed to add content type");
//                lws_return_http_status(wsi, HTTP_STATUS_INTERNAL_SERVER_ERROR, NULL);
//                goto try_to_reuse;
//            }

//            if (lws_add_http_header_by_name(wsi, (const unsigned char*) "Cache-Control", (const unsigned char*) "no-cache", strlen("no-cache"), &p, end) != 0)
//            {
//                LOG_ERROR("failed to add cache control");
//                lws_return_http_status(wsi, HTTP_STATUS_INTERNAL_SERVER_ERROR, NULL);
//                goto try_to_reuse;
//            }

//            if (lws_add_http_header_by_name(wsi, (const unsigned char*) "Pragma", (const unsigned char*) "no-cache", strlen("no-cache"), &p, end) != 0)
//            {
//                LOG_ERROR("failed to add cache control");
//                lws_return_http_status(wsi, HTTP_STATUS_INTERNAL_SERVER_ERROR, NULL);
//                goto try_to_reuse;
//            }

            if (is_get)
            {
                thread_sink->OnGet(conn->GetConnGUID());
            }

            if (lws_add_http_header_content_length(wsi, 5, &p, end))
            {
                return 1;
            }

            if (lws_finalize_http_header(wsi, &p, end) != 0)
            {
                LOG_ERROR("failed to finalize http header");
                lws_return_http_status(wsi, HTTP_STATUS_INTERNAL_SERVER_ERROR, NULL);
                goto try_to_reuse;
            }

            /*
                            * send the http headers...
                            * this won't block since it's the first payload sent
                            * on the connection since it was established
                            * (too small for partial)
                            *
                            * Notice they are sent using LWS_WRITE_HTTP_HEADERS
                            * which also means you can't send body too in one step,
                            * this is mandated by changes in HTTP2
                            */

            *p = '\0';
            lwsl_info("%s\n", rsp_header_buf + LWS_PRE);


            int n = lws_write(wsi, rsp_header_buf + LWS_PRE,
                              p - (rsp_header_buf + LWS_PRE),
                              LWS_WRITE_HTTP_HEADERS);
            if (n < 0)
            {
                return -1;
            }
            /*
             * book us a LWS_CALLBACK_HTTP_WRITEABLE callback
             */
            lws_callback_on_writable(wsi);
        }
        break;

        case LWS_CALLBACK_HTTP_WRITEABLE:
        {
            LOG_TRACE("LWS_CALLBACK_HTTP_WRITEABLE");

//            if (!pss)
//            {
//                break;
//            }

            /*
             * to keep this demo as simple as possible, each client has his
             * own private data and timer.
             */

//            p += lws_snprintf((char*)p, end - p,
//                              "data: %llu\x0d\x0a\x0d\x0a",
//                              (unsigned long long)time(NULL) -
//                              pss->established);

//            if (lws_write(wsi, (uint8_t*)start, lws_ptr_diff(p, start),
//                          LWS_WRITE_HTTP) != lws_ptr_diff(p, start))
//            {
//                return 1;
//            }

//            lws_set_timer_usecs(wsi, 3 * LWS_USEC_PER_SEC);
        }
        break;

        case LWS_CALLBACK_CLOSED_HTTP:
            LOG_TRACE("LWS_CALLBACK_CLOSED_HTTP");
            break;

        case LWS_CALLBACK_HTTP_BODY:
        {
            LOG_TRACE("LWS_CALLBACK_HTTP_BODY");
            LOG_DEBUG("in: " << (char*) in << ", len: " << len);
        }
        break;

        case LWS_CALLBACK_HTTP_BODY_COMPLETION:
        {
            LOG_TRACE("LWS_CALLBACK_HTTP_BODY_COMPLETION");
            LOG_DEBUG("in: " << (char*) in << ", len: " << len);
        }
        break;

        default:
        {
            return lws_callback_http_dummy(wsi, reason, user, in, len);
        }
        break;
    }

    return 0;

try_to_reuse:
    if (lws_http_transaction_completed(wsi))
    {
        return -1;
    }

    return 0;
}
}
}
