#include "http_msg_dispatcher.h"
#include "app_frame_conf_mgr_interface.h"
#include "http_thread_sink.h"
#include "http_threads_sync_facility.h"

#define GENERAL_REQ_PATH_PLACEHOLDER "*"

namespace http
{
MsgDispatcher::MsgDispatcher() : msg_handler_map_(), logic_ctx_(), embed_msg_handler_mgr_()
{
    thread_sink_ = NULL;
}

MsgDispatcher::~MsgDispatcher()
{
}

void MsgDispatcher::ReqCallback(struct evhttp_request* evhttp_req, void* arg, bool https)
{
    LOG_TRACE("MsgDispatcher::ReqCallback, evhttp req: " << evhttp_req << ", arg: " << arg);

    if (NULL == evhttp_req)
    {
        LOG_TRACE("evhttp req is null");
        return;
    }

    LOG_TRACE("http version, major: " << (int) evhttp_req->major << ", minor: " << (int) evhttp_req->minor
              << ", flags: " << evhttp_req->flags << ", uri: " << evhttp_request_get_uri(evhttp_req));

    CallbackArgs* callback_args = (CallbackArgs*) arg;
    callback_args->msg_dispatcher->OnHttpReq(evhttp_req, callback_args->msg_handler, https);
}

void MsgDispatcher::HttpReqCallback(struct evhttp_request* evhttp_req, void* arg)
{
    return MsgDispatcher::ReqCallback(evhttp_req, arg, false);
}

void MsgDispatcher::HttpsReqCallback(struct evhttp_request* evhttp_req, void* arg)
{
    return MsgDispatcher::ReqCallback(evhttp_req, arg, true);
}

void MsgDispatcher::GenericCallback(struct evhttp_request* evhttp_req, void* arg, bool https)
{
    LOG_TRACE("MsgDispatcher::GenericCallback, evhttp req: " << evhttp_req << ", arg: " << arg);

    if (NULL == evhttp_req)
    {
        LOG_TRACE("evhttp req is null");
        return;
    }

    LOG_TRACE("http version, major: " << (int) evhttp_req->major << ", minor: " << (int) evhttp_req->minor
              << ", flags: " << evhttp_req->flags << ", uri: " << evhttp_request_get_uri(evhttp_req));

    CallbackArgs* callback_args = (CallbackArgs*) arg;
    callback_args->msg_dispatcher->OnHttpReq(evhttp_req, callback_args->msg_handler, https);
}

void MsgDispatcher::HttpGenericCallback(struct evhttp_request* evhttp_req, void* arg)
{
    return MsgDispatcher::GenericCallback(evhttp_req, arg, false);
}

void MsgDispatcher::HttpsGenericCallback(struct evhttp_request* evhttp_req, void* arg)
{
    return MsgDispatcher::GenericCallback(evhttp_req, arg, true);
}

void MsgDispatcher::ConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg, bool https)
{
    LOG_TRACE("MsgDispatcher::ConnClosedCallback, evhttp conn: " << evhttp_conn << ", arg: " << arg);

    if (NULL == evhttp_conn)
    {
        LOG_TRACE("evhttp conn is null");
        return;
    }

    MsgDispatcher* http_msg_dispatcher = (MsgDispatcher*) arg;
    http_msg_dispatcher->OnHttpConnClosed(evhttp_conn, https);
}

void MsgDispatcher::HttpConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg)
{
    return MsgDispatcher::ConnClosedCallback(evhttp_conn, arg, false);
}

void MsgDispatcher::HttpsConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg)
{
    return MsgDispatcher::ConnClosedCallback(evhttp_conn, arg, true);
}

void MsgDispatcher::OnHttpReq(struct evhttp_request* evhttp_req, MsgHandlerInterface* http_msg_handler, bool https)
{
//    With HTTP/1.0 in order to keep a connection alive you must have the header Connection: keep-alive. Otherwise the server will close your connection after the request.
//    With HTTP/1.1, if the Connection header is not in the request, the server must assume keep-alive. HTTP/1.1 should use Connection: close if you want the server to close the connection.

    if (thread_sink_->GetThread()->IsStopping())
    {
        LOG_WARN("in stopping status, refuse http client data");
        evhttp_send_error(evhttp_req, HTTP_SERVUNAVAIL, NULL);
        return;
    }

    struct evhttp_connection* evhttp_conn = evhttp_request_get_connection(evhttp_req);
    if (NULL == evhttp_conn)
    {
        const int err = errno;
        LOG_ERROR("failed to get evhttp conn, errno: " << err << ", err msg: " << strerror(err));
        return;
    }

    LOG_TRACE("evhttp conn: " << evhttp_conn << ", evhttp req: " << evhttp_req); // TODO 同一个conn下的多个req测试

    app_frame::ConfMgrInterface* conf_mgr = thread_sink_->GetConfMgr();
    ConnCenterInterface* http_conn_center = thread_sink_->GetConnCenter();

    ConnInterface* http_conn = http_conn_center->GetConn(evhttp_conn);
    if (NULL == http_conn)
    {
        pthread_mutex_lock(&g_online_http_conn_count_mutex);

        const int http_connection_count_limit = conf_mgr->GetHttpConnCountLimit();
        if (http_connection_count_limit > 0 && g_online_http_conn_count >= http_connection_count_limit)
        {
            LOG_WARN("online http conn count: " << g_online_http_conn_count
                     << ", the limit is: " << http_connection_count_limit);
            evhttp_send_error(evhttp_req, HTTP_SERVUNAVAIL, NULL);
            evhttp_connection_free(evhttp_conn);
            pthread_mutex_unlock(&g_online_http_conn_count_mutex);
            return;
        }

        ++g_online_http_conn_count;

        if (g_online_http_conn_count > g_max_online_http_conn_count)
        {
            g_max_online_http_conn_count = g_online_http_conn_count;
            LOG_INFO("max online http conn count (global): " << g_max_online_http_conn_count);
        }

        pthread_mutex_unlock(&g_online_http_conn_count_mutex);

        http_conn = http_conn_center->CreateConn(thread_sink_->GetThread()->GetThreadIdx(), evhttp_conn, https);
        if (NULL == http_conn)
        {
            LOG_ERROR("failed to create http conn");
            evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
            evhttp_connection_free(evhttp_conn);
            return;
        }

        std::string addr;
        unsigned short port = 0;
        GetPeerAddr(addr, port, evhttp_req);

        http_conn->SetClientIpPort(addr.c_str(), port);

        if (!https)
        {
            evhttp_connection_set_closecb(evhttp_conn, MsgDispatcher::HttpConnClosedCallback, this);
        }
        else
        {
#if LIBEVENT_VERSION_NUMBER >= 0x2010500
            evhttp_connection_set_closecb(evhttp_conn, MsgDispatcher::HttpsConnClosedCallback, this);
#endif
        }
    }

    HandleHttpReq(&http_conn->GetConnGuid(), evhttp_req, https, http_msg_handler);
}

void MsgDispatcher::OnHttpConnClosed(struct evhttp_connection* evhttp_conn, bool https)
{
    LOG_TRACE("MsgDispatcher::OnHttpConnClosed, evhttp conn: " << evhttp_conn);

    ConnInterface* http_conn = thread_sink_->GetConnCenter()->GetConn(evhttp_conn);
    if (http_conn != NULL)
    {
        LOG_TRACE("http conn found, " << http_conn->GetConnGuid());
        thread_sink_->GetConnCenter()->RemoveConn(evhttp_conn);

        pthread_mutex_lock(&g_online_http_conn_count_mutex);
        --g_online_http_conn_count;
        pthread_mutex_unlock(&g_online_http_conn_count_mutex);
    }
    else
    {
        LOG_ERROR("http conn not found, evhttp conn: " << evhttp_conn);
    }
}

void MsgDispatcher::HandleHttpReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                                  MsgHandlerInterface* http_msg_handler)
{
    if (NULL == http_msg_handler)
    {
        LOG_WARN("there is no http msg handler" << evhttp_request_get_uri(evhttp_req));
        return;
    }

    app_frame::ConfMgrInterface* conf_mgr = thread_sink_->GetConfMgr();

    if (conf_mgr->HttpPrintParsedInfo())
    {
        PrintParsedInfo(evhttp_req);
    }

    struct timeval begin_time;
    gettimeofday(&begin_time, NULL);
    const long begin_millisecond = begin_time.tv_sec * 1000 + begin_time.tv_usec / 1000;

    KeyValMap http_header_map;

    struct evkeyvalq* input_headers = evhttp_request_get_input_headers(evhttp_req);
    if (input_headers != NULL)
    {
        for (struct evkeyval* header = input_headers->tqh_first; header != NULL; header = header->next.tqe_next)
        {
            LOG_TRACE(header->key << ": " << header->value);
            http_header_map[header->key] = Variant(Variant::TYPE_STR, header->value, strlen(header->value));
        }
    }

    const char* uri = evhttp_request_get_uri(evhttp_req);
    if (NULL == uri)
    {
        const int err = errno;
        LOG_ERROR("failed to get uri from evhttp req, errno: " << err << ", err msg: " << strerror(err));
        evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
        return;
    }

    LOG_TRACE("uri: " << uri);

    std::string path;
    std::string query;

    // uri可能做了encode，也可能没做，要跟发送方协商一致
    if (ParseUri(path, query, uri, conf_mgr->HttpDecodeUri()) != 0)
    {
        evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
        return;
    }

    if (strncmp(path.c_str(), http_msg_handler->GetHttpReqPath(), path.length()) != 0)
    {
        LOG_WARN("path not matched, 1: " << path << ", 2: " << http_msg_handler->GetHttpReqPath());
    }

    KeyValMap http_query_map;
    struct evkeyvalq query_kv;

    int ret = evhttp_parse_query_str(query.c_str(), &query_kv);
    if (ret != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to parse query, errno: " << err << ", err msg: " << strerror(err));
        evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
        return;
    }

    for (struct evkeyval* header = query_kv.tqh_first; header != NULL; header = header->next.tqe_next)
    {
        LOG_TRACE(header->key << ": " << header->value);
        http_query_map[header->key] = Variant(Variant::TYPE_STR, header->value, strlen(header->value));
    }

    evhttp_clear_headers(&query_kv);

    evhttp_cmd_type cmd_type = evhttp_request_get_command(evhttp_req);
    switch (cmd_type)
    {
        case EVHTTP_REQ_HEAD:
        {
            http_msg_handler->OnHttpHeadReq(conn_guid, evhttp_req, https, &http_header_map, &http_query_map);
        }
        break;

        case EVHTTP_REQ_GET:
        {
            http_msg_handler->OnHttpGetReq(conn_guid, evhttp_req, https, &http_header_map, &http_query_map);
        }
        break;

        case EVHTTP_REQ_POST:
        {
            HandleHttpPostReq(conn_guid, evhttp_req, https, &http_header_map, &http_query_map, http_msg_handler);
        }
        break;

        case EVHTTP_REQ_PUT:
        {
            HandleHttpPutReq(conn_guid, evhttp_req, https, &http_header_map, &http_query_map, http_msg_handler);
        }
        break;

        default:
        {
            LOG_ERROR("not supported http req type: " << cmd_type);
            evhttp_send_error(evhttp_req, HTTP_BADMETHOD, NULL);
        }
        break;
    }

    struct timeval end_time;
    gettimeofday(&end_time, NULL);
    const long end_millisecond = end_time.tv_sec * 1000 + end_time.tv_usec / 1000;

    if (http_msg_handler->GetHttpReqPath() != NULL)
    {
        LOG_TRACE("http msg process time: " << end_millisecond - begin_millisecond << " milliseconds. "
                  << conn_guid << ", path: " << http_msg_handler->GetHttpReqPath());
    }
    else
    {
        LOG_TRACE("http msg process time: " << end_millisecond - begin_millisecond << " milliseconds. "
                  << conn_guid);
    }
}

void MsgDispatcher::HandleHttpPostReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                                      const KeyValMap* http_header_map, const KeyValMap* http_query_map,
                                      MsgHandlerInterface* http_msg_handler)
{
    struct evbuffer* input_buf = evhttp_request_get_input_buffer(evhttp_req);
    if (NULL == input_buf)
    {
        LOG_ERROR("input buf is null");
        evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
        return;
    }

    const size_t input_data_len = evbuffer_get_length(input_buf);
    LOG_TRACE("input data len: " << input_data_len);

    if (input_data_len <= 0)
    {
        LOG_ERROR("invalid input data len: " << input_data_len);
        evhttp_send_error(evhttp_req, HTTP_BADREQUEST, NULL);
        return;
    }

    const char* input_data = (const char*) evbuffer_pullup(input_buf, input_data_len);
    if (NULL == input_data)
    {
        LOG_ERROR("failed to get input data");
        evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
        return;
    }

    http_msg_handler->OnHttpPostReq(conn_guid, evhttp_req, https, http_header_map, http_query_map,
                                    input_data, input_data_len);
}

void MsgDispatcher::HandleHttpPutReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                                     const KeyValMap* http_header_map, const KeyValMap* http_query_map,
                                     MsgHandlerInterface* http_msg_handler)
{
    struct evbuffer* input_buf = evhttp_request_get_input_buffer(evhttp_req);
    if (NULL == input_buf)
    {
        LOG_ERROR("input buf is null");
        evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
        return;
    }

    const size_t input_data_len = evbuffer_get_length(input_buf);
    LOG_TRACE("input data len: " << input_data_len);

    if (input_data_len <= 0)
    {
        LOG_ERROR("invalid input data len: " << input_data_len);
        evhttp_send_error(evhttp_req, HTTP_BADREQUEST, NULL);
        return;
    }

    const char* input_data = (const char*) evbuffer_pullup(input_buf, input_data_len);
    if (NULL == input_data)
    {
        LOG_ERROR("failed to get input data");
        evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
        return;
    }

    http_msg_handler->OnHttpPutReq(conn_guid, evhttp_req, https, http_header_map, http_query_map,
                                   input_data, input_data_len);
}

int MsgDispatcher::Initialize(const void* ctx)
{
    embed_msg_handler_mgr_.SetThreadSink(thread_sink_);

    memset(&logic_ctx_, 0, sizeof(LogicCtx));
    logic_ctx_.conn_center = thread_sink_->GetConnCenter();
    logic_ctx_.msg_dispatcher = this;
    logic_ctx_.scheduler = thread_sink_->GetScheduler();

    if (embed_msg_handler_mgr_.Initialize(&logic_ctx_) != 0)
    {
        return -1;
    }

    return 0;
}

void MsgDispatcher::Finalize()
{
    embed_msg_handler_mgr_.Finalize();
    msg_handler_map_.clear();
}

int MsgDispatcher::Activate()
{
    if (embed_msg_handler_mgr_.Activate() != 0)
    {
        return -1;
    }

    return 0;
}

void MsgDispatcher::Freeze()
{
    embed_msg_handler_mgr_.Freeze();
}

void MsgDispatcher::PrintParsedInfo(struct evhttp_request* req)
{
    struct evkeyvalq* input_headers = evhttp_request_get_input_headers(req);
    if (input_headers != NULL)
    {
        for (struct evkeyval* header = input_headers->tqh_first; header != NULL; header = header->next.tqe_next)
        {
            LOG_INFO(header->key << ": " << header->value);
        }

        const char* x_forwarded_for = evhttp_find_header(input_headers, "X-Forwarded-For");
        if (x_forwarded_for != NULL)
        {
            LOG_INFO("X-Forwarded-For: " << x_forwarded_for);
        }
        else
        {
            LOG_INFO("X-Forwarded-For:");
        }
    }
    else
    {
        LOG_INFO("input headers is null");
    }

    const char* uri = evhttp_request_get_uri(req);
    if (uri != NULL)
    {
        LOG_INFO("uri: " << uri);
    }
    else
    {
        LOG_INFO("uri:");
    }

    size_t decoded_uri_len = 0;
    char* decoded_uri = evhttp_uridecode(uri, 0, &decoded_uri_len);
    if (NULL == decoded_uri)
    {
        const int err = errno;
        LOG_ERROR("failed to decode uri: " << uri << ", errno: " << err << ", err msg: " << strerror(err));
        return;
    }

    LOG_INFO("decoded uri: " << decoded_uri);

    struct evhttp_uri* struct_uri = NULL;
    const char* host = NULL;
    int port = -1;
    const char* scheme = NULL;
    const char* user_info = NULL;
    const char* path = NULL;
    const char* query = NULL;
    const char* fragment = NULL;

    struct_uri = evhttp_uri_parse_with_flags(decoded_uri, EVHTTP_URI_NONCONFORMANT);
    if (NULL == struct_uri)
    {
        const int err = errno;
        LOG_ERROR("failed to parse uri: " << decoded_uri << ", errno: " << err << ", err msg: " << strerror(err));
        goto err_out;
    }

    host = evhttp_uri_get_host(struct_uri);
    if (host != NULL)
    {
        LOG_INFO("host: " << host);
    }
    else
    {
        LOG_INFO("host:");
    }

    port = evhttp_uri_get_port(struct_uri);
    LOG_INFO("port: " << port);

    scheme = evhttp_uri_get_scheme(struct_uri);
    if (scheme != NULL)
    {
        LOG_INFO("scheme: " << scheme);
    }
    else
    {
        LOG_INFO("scheme:");
    }

    user_info = evhttp_uri_get_userinfo(struct_uri);
    if (user_info != NULL)
    {
        LOG_INFO("user info: " << user_info);
    }
    else
    {
        LOG_INFO("user info:");
    }

    path = evhttp_uri_get_path(struct_uri);
    if (path != NULL)
    {
        LOG_INFO("path: " << path);
    }
    else
    {
        LOG_INFO("path:");
    }

    query = evhttp_uri_get_query(struct_uri);
    if (query != NULL)
    {
        LOG_INFO("query: " << query);

        struct evkeyvalq params;

        int ret = evhttp_parse_query_str(query, &params);
        if (ret != 0)
        {
            const int err = errno;
            LOG_ERROR("failed to parse query, errno: " << err << ", err msg: " << strerror(err));
        }
        else
        {
            for (evkeyval* param = params.tqh_first; param; param = param->next.tqe_next)
            {
                LOG_INFO(param->key << ": " << param->value);
            }

            evhttp_clear_headers(&params);
        }
    }
    else
    {
        LOG_INFO("query:");
    }

    fragment = evhttp_uri_get_fragment(struct_uri);
    if (fragment != NULL)
    {
        LOG_INFO("fragment: " << fragment);
    }
    else
    {
        LOG_INFO("fragment:");
    }

    evhttp_uri_free(struct_uri);
    free(decoded_uri);
    return;

err_out:
    if (decoded_uri != NULL)
    {
        free(decoded_uri);
    }

    if (struct_uri != NULL)
    {
        evhttp_uri_free(struct_uri);
    }

    return;
}

int MsgDispatcher::GetPeerAddr(std::string& addr, unsigned short& port, struct evhttp_request* evhttp_req)
{
    struct evhttp_connection* evhttp_conn = evhttp_request_get_connection(evhttp_req);
    if (NULL == evhttp_conn)
    {
        const int err = errno;
        LOG_ERROR("failed to get evhttp conn, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    char* peer_addr;
    ev_uint16_t peer_port;
    evhttp_connection_get_peer(evhttp_conn, &peer_addr, &peer_port);
    LOG_INFO("peer addr: " << peer_addr << ", peer port: " << peer_port);

    addr = peer_addr;
    port = peer_port;

    struct evkeyvalq* headers = evhttp_request_get_input_headers(evhttp_req);
    if (headers != NULL)
    {
        const char* x_forwarded_for = evhttp_find_header(headers, "X-Forwarded-For");
        if (x_forwarded_for != NULL)
        {
            LOG_INFO("X-Forwarded-For: " << x_forwarded_for);
            const char* p = strchr(x_forwarded_for, ',');
            if (NULL == p)
            {
                addr.assign(x_forwarded_for);
            }
            else
            {
                addr.assign(x_forwarded_for, p - x_forwarded_for);
            }
        }
    }

    return 0;
}

int MsgDispatcher::ParseUri(std::string& path, std::string& query, const char* uri, bool need_decode)
{
    /**
     * 测试发现，客户端如果是对k=v的v单独做url encode，则无论服务端是否配置了need_decode，都能正确decode。
     */

    size_t decoded_uri_len = 0;
    char* decoded_uri = NULL;
    const char* uri_ptr = uri;
    struct evhttp_uri* struct_uri = NULL;
    const char* path_ptr = NULL;
    const char* query_ptr = NULL;

    if (need_decode)
    {
        decoded_uri = evhttp_uridecode(uri, 0, &decoded_uri_len);
        if (NULL == decoded_uri)
        {
            LOG_ERROR("failed to decode uri: " << uri);
            return -1;
        }

        LOG_TRACE("decoded uri: " << decoded_uri);
        uri_ptr = decoded_uri;
    }

    struct_uri = evhttp_uri_parse_with_flags(uri_ptr, EVHTTP_URI_NONCONFORMANT);
    if (NULL == struct_uri)
    {
        LOG_ERROR("failed to parse uri: " << uri_ptr);
        goto err_out;
    }

    path_ptr = evhttp_uri_get_path(struct_uri);
    if (path_ptr != NULL)
    {
        path.assign(path_ptr);
    }

    query_ptr = evhttp_uri_get_query(struct_uri);
    if (query_ptr != NULL)
    {
        query.assign(query_ptr);
    }

    evhttp_uri_free(struct_uri);
    free(decoded_uri);
    return 0;

err_out:
    if (decoded_uri != NULL)
    {
        free(decoded_uri);
    }

    if (struct_uri != NULL)
    {
        evhttp_uri_free(struct_uri);
    }

    return -1;
}

int MsgDispatcher::AttachMsgHandler(const char* path, MsgHandlerInterface* http_msg_handler)
{
    if (NULL == path || strlen(path) < 1)
    {
        return -1;
    }

    MsgHandlerMap::iterator it = msg_handler_map_.find(path);
    if (it != msg_handler_map_.end())
    {
        LOG_ERROR("http req path " << path << " already exist");
        return -1;
    }

    msg_handler_map_[path].msg_dispatcher = this;
    msg_handler_map_[path].msg_handler = http_msg_handler;

    std::string path_with_slash = path;
    path_with_slash.append("/");

    msg_handler_map_[path_with_slash].msg_dispatcher = this;
    msg_handler_map_[path_with_slash].msg_handler = http_msg_handler;

    if (thread_sink_->GetHttpServer() != NULL)
    {
        if (evhttp_set_cb(thread_sink_->GetHttpServer(), path,
                          MsgDispatcher::HttpReqCallback, &msg_handler_map_[path]) != 0)
        {
            const int err = errno;
            LOG_ERROR("evhttp_set_cb failed, errno: " << err << ", err: " << strerror(err));

            msg_handler_map_.erase(path);
            return -1;
        }

        if (evhttp_set_cb(thread_sink_->GetHttpServer(), path_with_slash.c_str(),
                          MsgDispatcher::HttpReqCallback, &msg_handler_map_[path_with_slash]) != 0)
        {
            const int err = errno;
            LOG_ERROR("evhttp_set_cb failed, errno: " << err << ", err: " << strerror(err));

            msg_handler_map_.erase(path);
            return -1;
        }
    }

    if (thread_sink_->GetHttpsServer() != NULL)
    {
        if (evhttp_set_cb(thread_sink_->GetHttpsServer(), path,
                          MsgDispatcher::HttpsReqCallback, &msg_handler_map_[path]) != 0)
        {
            const int err = errno;
            LOG_ERROR("evhttp_set_cb failed, errno: " << err << ", err: " << strerror(err));

            if (thread_sink_->GetHttpServer() != NULL)
            {
                evhttp_del_cb(thread_sink_->GetHttpServer(), path);
            }

            msg_handler_map_.erase(path);
            return -1;
        }

        if (evhttp_set_cb(thread_sink_->GetHttpsServer(), path_with_slash.c_str(),
                          MsgDispatcher::HttpsReqCallback, &msg_handler_map_[path_with_slash]) != 0)
        {
            const int err = errno;
            LOG_ERROR("evhttp_set_cb failed, errno: " << err << ", err: " << strerror(err));

            if (thread_sink_->GetHttpServer() != NULL)
            {
                evhttp_del_cb(thread_sink_->GetHttpServer(), path);
            }

            msg_handler_map_.erase(path);
            return -1;
        }
    }

    LOG_INFO("attach http msg handler ok, path: " << path);
    return 0;
}

void MsgDispatcher::DetachMsgHandler(const char* path)
{
    if (NULL == path)
    {
        return;
    }

    MsgHandlerMap::iterator it = msg_handler_map_.find(path);
    if (it != msg_handler_map_.end())
    {
        if (thread_sink_->GetHttpServer() != NULL)
        {
            evhttp_del_cb(thread_sink_->GetHttpServer(), path);
        }

        if (thread_sink_->GetHttpsServer() != NULL)
        {
            evhttp_del_cb(thread_sink_->GetHttpsServer(), path);
        }

        msg_handler_map_.erase(it);
        return;
    }
}

int MsgDispatcher::AttachGeneralMsgHandler(MsgHandlerInterface* general_req_handler)
{
    MsgHandlerMap::iterator it = msg_handler_map_.find(GENERAL_REQ_PATH_PLACEHOLDER);
    if (it != msg_handler_map_.end())
    {
        LOG_ERROR("general http msg handler already exist");
        return -1;
    }

    msg_handler_map_[GENERAL_REQ_PATH_PLACEHOLDER].msg_dispatcher = this; // general req placeholder
    msg_handler_map_[GENERAL_REQ_PATH_PLACEHOLDER].msg_handler = general_req_handler;

    if (thread_sink_->GetHttpServer() != NULL)
    {
        evhttp_set_gencb(thread_sink_->GetHttpServer(), MsgDispatcher::HttpGenericCallback,
                         &msg_handler_map_[GENERAL_REQ_PATH_PLACEHOLDER]);
    }

    if (thread_sink_->GetHttpsServer() != NULL)
    {
        evhttp_set_gencb(thread_sink_->GetHttpsServer(), MsgDispatcher::HttpsGenericCallback,
                         &msg_handler_map_[GENERAL_REQ_PATH_PLACEHOLDER]);
    }

    LOG_INFO("attach http general msg handler ok");
    return 0;
}

void MsgDispatcher::DetachGeneralMsgHandler()
{
    if (thread_sink_->GetHttpServer() != NULL)
    {
        evhttp_set_gencb(thread_sink_->GetHttpServer(), NULL, NULL);
    }

    if (thread_sink_->GetHttpsServer() != NULL)
    {
        evhttp_set_gencb(thread_sink_->GetHttpsServer(), NULL, NULL);
    }

    msg_handler_map_.erase(GENERAL_REQ_PATH_PLACEHOLDER);
}
}
