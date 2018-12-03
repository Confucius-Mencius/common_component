#include "http_client.h"

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
#include <event2/bufferevent_ssl.h>
#include <openssl/err.h>
#endif

#include "log_util.h"
#include "str_util.h"

//
// evhttp_connection对象只能异步释放，参见https://github.com/libevent/libevent/issues/115
// If you wanted to free the connection object you could do so after the base is done dispatching,
// or you could use the http_request_done to queue the free operation on another thread or event processor to have it free'd asynchronously.
//

namespace http
{
void Client::HttpConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg)
{
    Client* client = (Client*) arg;
    LOG_TRACE("Client::HttpConnClosedCallback, evhttp conn: " << evhttp_conn << ", client: " << client << ", "
              << client->peer_);
}

void Client::HttpsConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg)
{
    Client* client = (Client*) arg;
    LOG_TRACE("Client::HttpConnClosedCallback, evhttp conn: " << evhttp_conn << ", client: " << client << ", "
              << client->peer_);
}

void Client::HttpReqDoneCallback(struct evhttp_request* evhttp_req, void* arg)
{
    const int err = EVUTIL_SOCKET_ERROR();
    CallbackArg* callback_arg = (CallbackArg*) arg;
    LOG_TRACE("Client::HttpReqDoneCallback, client: " << callback_arg->http_client << ", "
              << callback_arg->http_client->peer_);

    if (NULL == evhttp_req)
    {
        // 对端在处理req时挂掉或者超时后还未返回,则会走到这里
        LOG_TRACE("evhttp req is null");

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
        if (callback_arg->https)
        {
            unsigned long osl_err = 0;
            bool printed_err = false;
            while ((osl_err = bufferevent_get_openssl_error(callback_arg->http_client->buf_event_)))
            {
                char buf[128] = "";
                ERR_error_string_n(osl_err, buf, sizeof(buf));
                LOG_ERROR("Client::HttpReqDoneCallback, https, " << buf << ", " << callback_arg->http_client->peer_);
                printed_err = true;
            }

            if (!printed_err && err != 0)
            {
                LOG_ERROR("socket error, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err) << ", "
                          << callback_arg->http_client->peer_);
            }
        }
#endif

        const TransId& trans_id = callback_arg->trans_id;
        TransCtx* trans_ctx = callback_arg->http_client->client_center_ctx_->trans_center->GetTransCtx(trans_id);
        if (NULL == trans_ctx)
        {
            LOG_WARN("failed to get trans ctx by trans id: " << trans_id << ", maybe canceled, "
                     << callback_arg->http_client->peer_);
        }
        else
        {
            if (trans_ctx->sink != NULL)
            {
                trans_ctx->sink->OnClosed(trans_id, callback_arg->http_client->GetPeer(), trans_ctx->async_data,
                                          trans_ctx->async_data_len);
            }

            callback_arg->http_client->client_center_ctx_->trans_center->CancelTrans(trans_id);
        }
    }
    else
    {
        callback_arg->http_client->OnHttpReqDone(callback_arg->trans_id, callback_arg->http_client->GetPeer(),
                callback_arg->https, evhttp_req);
    }

    callback_arg->http_client->callback_arg_set_.erase(callback_arg);
    callback_arg->Release();

    // 添加一个定时器,异步释放evhttp_connection对象
//    struct timeval tv;
//    interval.tv_sec = 0;
//    interval.tv_usec = 1000;
//
//    struct event_base* ev_base = callback_arg->http_client->client_center_ctx_->thread_ev_base;
//
//    callback_arg->cleanup_event = event_new(ev_base, -1, EV_PERSIST, Client::OnConnCleanupEvent, callback_arg);
//    if (NULL == callback_arg->cleanup_event)
//    {
//        const int err = errno;
//        LOG_ERROR("failed to create http conn cleanup timer, errno: " << err << ", err msg: " << strerror(err));
//        return;
//    }
//
//    if (event_add(callback_arg->cleanup_event, &interval) != 0)
//    {
//        const int err = errno;
//        LOG_ERROR("failed to add http conn cleanup timer, errno: " << err << ", err msg: " << strerror(err));
//        event_free(callback_arg->cleanup_event);
//        return;
//    }

    // 看libevent源码，回调完后会自动释放evhttp_req。
}

#if LIBEVENT_VERSION_NUMBER >= 0x2010500

void Client::HttpReqErrorCallback(evhttp_request_error err, void* arg)
{
    LOG_TRACE("Client::HttpReqErrorCallback, err: " << err << ", arg: " << arg);
}

#endif

//void Client::OnConnCleanupEvent(int sock_fd, short which, void* arg)
//{
//    // 定时器socket fd为-1，which为1，表示定时器到了
//    CallbackArg* callback_arg = (CallbackArg*) arg;
//    Client* client = callback_arg->http_client;
//    LOG_TRACE("Client::OnConnCleanupEvent, which: " << which << ", client: " << client);
//
//    client->callback_arg_set_.erase(callback_arg);
//    callback_arg->Release();
//
//    if (0 == client->callback_arg_set_.size())
//    {
//        client->client_center_->RemoveClient(client->peer_);
//    }
//}

Client::Client() : peer_(), callback_arg_set_()
{
    client_center_ = NULL;
    client_center_ctx_ = NULL;
    evhttp_conn_ = NULL;
    sctx_ = NULL;
    buf_event_ = NULL;
    evhttps_conn_ = NULL;
}

Client::~Client()
{
}

const char* Client::GetVersion() const
{
    return nullptr;
}

const char* Client::GetLastErrMsg() const
{
    return nullptr;
}

void Client::Release()
{
    delete this;
}

int Client::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    client_center_ctx_ = (ClientCenterCtx*) ctx;
    return 0;
}

void Client::Finalize()
{
    for (CallbackArgSet::iterator it = callback_arg_set_.begin(); it != callback_arg_set_.end(); ++it)
    {
        (*it)->Release();
    }

    callback_arg_set_.clear();

    if (evhttp_conn_ != NULL)
    {
        evhttp_connection_free(evhttp_conn_);
        evhttp_conn_ = NULL;
    }

    if (evhttps_conn_ != NULL)
    {
        evhttp_connection_free(evhttps_conn_); // 看libevent源码，evhttp_connection_free中会释放buf_event_
        evhttps_conn_ = NULL;
    }

    if (sctx_ != NULL)
    {
        SSL_CTX_free(sctx_);
        sctx_ = NULL;
    }
}

int Client::Activate()
{
    if (CreateEvHttpConn(peer_) != 0)
    {
        return -1;
    }

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    if (CreateEvHttpsConn(peer_) != 0)
    {
        return -1;
    }
#else
    LOG_WARN("do not support https");
#endif

    return 0;
}

void Client::Freeze()
{
}

TransId Client::Head(const HeadParams& params, const base::AsyncCtx* async_ctx)
{
    return INVALID_TRANS_ID;
}

TransId Client::Get(const GetParams& params, const base::AsyncCtx* async_ctx)
{
    // 这里把timeout设置为0，使用http自己的timeout机制
    TransCtx trans_ctx;
    trans_ctx.peer = peer_;
    trans_ctx.timeout_sec = 0;
    trans_ctx.passback = 0;

    if (async_ctx != NULL)
    {
        trans_ctx.sink = async_ctx->sink;
        trans_ctx.async_data = (char*) async_ctx->async_data;
        trans_ctx.async_data_len = async_ctx->async_data_len;
    }

    TransId trans_id = client_center_ctx_->trans_center->RecordTransCtx(&trans_ctx);
    if (INVALID_TRANS_ID == trans_id)
    {
        return trans_id;
    }

    // 这里没有用total_retry参数，使用http自己的max_retry机制
    if (DoHttpReq(trans_id, params.uri, params.uri_len, params.need_encode, params.header_map, NULL,
                  0, params.https) != 0)
    {
        client_center_ctx_->trans_center->CancelTrans(trans_id);
        return INVALID_TRANS_ID;
    }

    return trans_id;
}

TransId Client::Post(const PostParams& params, const base::AsyncCtx* async_ctx)
{
    // 这里把timeout设置为0，使用http自己的timeout机制
    TransCtx trans_ctx;
    trans_ctx.peer = peer_;
    trans_ctx.timeout_sec = 0;
    trans_ctx.passback = 0;

    if (async_ctx != NULL)
    {
        trans_ctx.sink = async_ctx->sink;
        trans_ctx.async_data = (char*) async_ctx->async_data;
        trans_ctx.async_data_len = async_ctx->async_data_len;
    }

    TransId trans_id = client_center_ctx_->trans_center->RecordTransCtx(&trans_ctx);
    if (INVALID_TRANS_ID == trans_id)
    {
        return trans_id;
    }

    // 这里没有用total_retry参数，使用http自己的max_retry机制
    if (DoHttpReq(trans_id, params.get_params.uri, params.get_params.uri_len, params.get_params.need_encode,
                  params.get_params.header_map, params.data, params.data_len, params.get_params.https) != 0)
    {
        client_center_ctx_->trans_center->CancelTrans(trans_id);
        return INVALID_TRANS_ID;
    }

    return trans_id;
}

void Client::OnHttpReqDone(TransId trans_id, const Peer& peer, bool https, struct evhttp_request* evhttp_req)
{
    LOG_TRACE("Client::OnHttpReqDone, trans id: " << trans_id << ", https: " << https
              << ", evhttp req: " << evhttp_req);

    const int http_code = evhttp_request_get_response_code(evhttp_req);
    LOG_TRACE("http code: " << http_code);

    TransCtx* trans_ctx = client_center_ctx_->trans_center->GetTransCtx(trans_id);
    if (NULL == trans_ctx)
    {
        LOG_WARN("failed to get trans ctx by trans id: " << trans_id << ", maybe canceled");
        return;
    }

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    if (EVREQ_HTTP_TIMEOUT == http_code)
#else
    if (0 == http_code)
#endif
    {
        if (trans_ctx->sink != NULL)
        {
            trans_ctx->sink->OnTimeout(trans_id, peer, trans_ctx->async_data, trans_ctx->async_data_len);
        }

        client_center_ctx_->trans_center->CancelTrans(trans_id);
        return;
    }

    char* status_line = NULL; // req->response_code_line;
    (void) status_line;

    KeyValMap header_map;

    struct evkeyvalq* input_headers = evhttp_request_get_input_headers(evhttp_req);
    if (input_headers != NULL)
    {
        for (struct evkeyval* header = input_headers->tqh_first; header != NULL; header = header->next.tqe_next)
        {
            header_map.insert(
                KeyValMap::value_type(header->key, Variant(Variant::TYPE_STR, header->value, strlen(header->value))));
        }
    }

    struct evbuffer* input_buf = evhttp_request_get_input_buffer(evhttp_req);
    const size_t rsp_body_len = evbuffer_get_length(input_buf);
    char* rsp_body = NULL;

    if (rsp_body_len > 0)
    {
        rsp_body = new char[rsp_body_len + 1];
        evbuffer_remove(input_buf, rsp_body, rsp_body_len);
        rsp_body[rsp_body_len] = '\0';
    }

    if (trans_ctx->sink != NULL)
    {
        Rsp http_rsp;
        http_rsp.https = https;
        http_rsp.http_code = http_code;
        http_rsp.status_line = status_line;
        http_rsp.header_map = &header_map;
        http_rsp.rsp_body = rsp_body;
        http_rsp.rsp_body_len = rsp_body_len;

        trans_ctx->sink->OnRecvRsp(trans_id, peer, &http_rsp, trans_ctx->async_data, trans_ctx->async_data_len);
    }

    client_center_ctx_->trans_center->CancelTrans(trans_id);

    if (rsp_body != NULL)
    {
        delete[] rsp_body;
    }
}

int Client::CreateEvHttpConn(const Peer& peer)
{
    // header size和body size的大小均不限制，重连间隔初始为2秒，以后每次都翻倍。
    evhttp_conn_ = evhttp_connection_base_new(client_center_ctx_->thread_ev_base, NULL, peer.addr.c_str(), peer.port);
    if (NULL == evhttp_conn_)
    {
        const int err = errno;
        LOG_ERROR("failed to create evhttp conn to " << peer << ", errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (client_center_ctx_->http_conn_timeout > 0)
    {
        LOG_TRACE("http conn timeout: " << client_center_ctx_->http_conn_timeout);
        // timeout跟keep alive相关，在这段时间内如果没有消息传递，则关闭连接 TODO 待验证
        evhttp_connection_set_timeout(evhttp_conn_, client_center_ctx_->http_conn_timeout);
    }

    if (client_center_ctx_->http_conn_max_retry > 0)
    {
        LOG_TRACE("http conn max retry: " << client_center_ctx_->http_conn_max_retry);
        evhttp_connection_set_retries(evhttp_conn_, client_center_ctx_->http_conn_max_retry);
    }

    // 观察libevent的处理流程，发现一段时间后该连接上没有数据传输则会进到回调中。回调中不用做任何处理，下次请求仍可以复用该连接。
    evhttp_connection_set_closecb(evhttp_conn_, Client::HttpConnClosedCallback, this);

    LOG_TRACE("evhttp conn: " << evhttp_conn_ << ", flags: " << evhttp_connection_get_flags(evhttp_conn_));
    return 0;
}

#if LIBEVENT_VERSION_NUMBER >= 0x2010500

int Client::CreateEvHttpsConn(const Peer& peer)
{
    /* An OpenSSL context holds data that new SSL connections will
     * be created from. */
    sctx_ = SSL_CTX_new(SSLv23_client_method());
    if (NULL == sctx_)
    {
        const int err = errno;
        LOG_ERROR("SSL_CTX_new failed, errno: " << err << ", err msg: " << strerror(err));;
        return -1;
    }

    /* Find the certificate authority (which we will use to
     * validate the server) and add it to the context. */
//    SSL_CTX_load_verify_locations(sctx, "certificate-authorities.pem", NULL); // TODO 这个应该是客户端的校验，这里先注释掉

    SSL_CTX_set_verify(sctx_, SSL_VERIFY_NONE, NULL); // 如果为SSL_VERIFY_PEER表示客户端会做校验

    /* Create a new SSL connection from our SSL context */
    SSL* ssl = SSL_new(sctx_);
    if (NULL == ssl)
    {
        const int err = errno;
        LOG_ERROR("SSL_new failed, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    /* Now wrap the SSL connection in an SSL bufferevent */
    buf_event_ = bufferevent_openssl_socket_new(client_center_ctx_->thread_ev_base, -1, ssl, BUFFEREVENT_SSL_CONNECTING,
                 0 | BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    if (NULL == buf_event_)
    {
        const int err = errno;
        LOG_ERROR("failed to create buffer event openssl socket, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    /* Newly-added function in libevent 2.1 which allows us to specify
     * our own bufferevent (e. g. one with SSL) when creating a new
     * HTTP connection.  Sorry, not available in libevent 2.0. */
    // header size和body size的大小均不限制，重连间隔初始为2秒，以后每次都翻倍。
    evhttps_conn_ = evhttp_connection_base_bufferevent_new(client_center_ctx_->thread_ev_base, 0, buf_event_,
                    peer.addr.c_str(), peer.port);
    if (NULL == evhttps_conn_)
    {
        const int err = errno;
        LOG_ERROR("failed to create evhttps conn to " << peer << ", errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (client_center_ctx_->http_conn_timeout > 0)
    {
        LOG_TRACE("https conn timeout: " << client_center_ctx_->http_conn_timeout);
        // timeout跟keep alive相关，在这段时间内如果没有消息传递，则关闭连接 TODO 待验证
        evhttp_connection_set_timeout(evhttps_conn_, client_center_ctx_->http_conn_timeout);
    }

    if (client_center_ctx_->http_conn_max_retry > 0)
    {
        LOG_TRACE("https conn max retry: " << client_center_ctx_->http_conn_max_retry);
        evhttp_connection_set_retries(evhttps_conn_, client_center_ctx_->http_conn_max_retry);
    }

    // 观察libevent的处理流程，发现一段时间后该连接上没有数据传输则会进到回调中。回调中不用做任何处理，下次请求仍可以复用该连接。
    evhttp_connection_set_closecb(evhttps_conn_, Client::HttpsConnClosedCallback, this);

    LOG_TRACE("evhttps conn: " << evhttps_conn_ << ", flags: " << evhttp_connection_get_flags(evhttps_conn_));
    return 0;
}

#endif

int Client::DoHttpReq(TransId trans_id, const char* uri, int uri_len, bool need_encode, const KeyValMap* header_map,
                      const void* data, size_t data_len, bool https)
{
    if (NULL == uri)
    {
        return -1;
    }

    LOG_TRACE("trans id: " << trans_id << ", " << peer_ << ", uri: " << uri << ", uri len: " << uri_len
              << ", need encode: " << need_encode << ", data len: " << data_len << ", https: " << https);

    char* req_uri = (char*) uri;
    char* encoded_uri = NULL;
    CallbackArg* callback_arg = NULL;
    struct evhttp_request* evhttp_req = NULL;
    struct evkeyvalq* output_headers = NULL;
    evhttp_cmd_type cmd_type = EVHTTP_REQ_GET;
    struct evhttp_connection* evhttp_conn = (https ? evhttps_conn_ : evhttp_conn_);

    std::stringstream host_stream;
    host_stream.str("");
    host_stream << peer_.addr << ":" << peer_.port;

    if (need_encode)
    {
        encoded_uri = evhttp_uriencode(uri, uri_len, 0);
        if (NULL == encoded_uri)
        {
            LOG_ERROR("failed to encode uri: " << uri << ", len: " << uri_len);
            return -1;
        }

        req_uri = encoded_uri;
    }

    callback_arg = CallbackArg::Create();
    if (NULL == callback_arg)
    {
        const int err = errno;
        LOG_ERROR("failed to create callback arg, errno: " << err << ", err msg: " << strerror(err));
        goto err_out;
    }

    callback_arg->http_client = this;
    callback_arg->https = https;
    callback_arg->trans_id = trans_id;

    LOG_TRACE("before evhttp_request_new");

    evhttp_req = evhttp_request_new(Client::HttpReqDoneCallback, callback_arg); // 处理完成后libevent会自动释放req
    if (NULL == evhttp_req)
    {
        const int err = errno;
        LOG_ERROR("failed to create evhttp req, errno: " << err << ", err msg: " << strerror(err));
        goto err_out;
    }

    LOG_TRACE("after evhttp_request_new");

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    // 注意：注册这个回调纯粹为了观察libevent的处理流程，无实际意义
    evhttp_request_set_error_cb(evhttp_req, Client::HttpReqErrorCallback);
#endif

    // http headers
    output_headers = evhttp_request_get_output_headers(evhttp_req);

    if (header_map != NULL)
    {
        char buf[256] = "";

        for (KeyValMap::const_iterator it = header_map->begin(); it != header_map->end(); ++it)
        {
            if (it->first == "Host" || it->first == "HOST")
            {
                continue;
            }

            memset(buf, 0, sizeof(buf));
            it->second.ToString(buf, sizeof(buf));
            evhttp_add_header(output_headers, it->first.c_str(), buf);
        }
    }

    // 在http 1.1中不能缺失host字段,如果缺失, 服务器返回400 bad request，http1.1中不能缺失host字段，但host字段可以是空值。
    // 在http 1.0中可以缺失host字段。
    // RFC 2616中关于host字段的说明如下：
    // A client MUST include a Host header field in all HTTP/1.1 request messages . If the requested URI does not include an Internet host name
    // for the service being requested, then the Host header field MUST be given with an empty value. An HTTP/1.1 proxy MUST ensure that any request message
    // it forwards does contain an appropriate Host header field that identifies the service being requested by the proxy. All Internet-based HTTP/1.1 servers
    // MUST respond with a 400 (Bad Request) status code to any HTTP/1.1 request message which lacks a Host header field.
    evhttp_add_header(output_headers, "Host", host_stream.str().c_str());

    for (struct evkeyval* header = output_headers->tqh_first; header != NULL; header = header->next.tqe_next)
    {
        LOG_TRACE(header->key << ": " << header->value);
    }

    // http body
    if (data != NULL && data_len > 0)
    {
        // 有些web服务器需要根据Content-Type对post内容做解析，所以这里必须加上Content-Type
        if (NULL == evhttp_find_header(output_headers, "Content-Type"))
        {
            evhttp_add_header(output_headers, "Content-Type", "application/x-www-form-urlencoded");
        }

        cmd_type = EVHTTP_REQ_POST;

        struct evbuffer* output_buf = evhttp_request_get_output_buffer(evhttp_req);
        if (NULL == output_buf)
        {
            LOG_ERROR("failed to get output buf");
            goto err_out;
        }

        if (evbuffer_add(output_buf, data, data_len) != 0)
        {
            const int err = errno;
            LOG_ERROR("failed to add passback， errno: " << err << ", err msg: " << strerror(err));
            goto err_out;
        }

        char data_len_buf[32] = "";
        StrPrintf(data_len_buf, sizeof(data_len_buf), "%lu", data_len);

        evhttp_remove_header(output_headers, "Content-Length");
        evhttp_add_header(output_headers, "Content-Length", data_len_buf);
    }

    LOG_TRACE("before evhttp_make_request");

    if (evhttp_make_request(evhttp_conn, evhttp_req, cmd_type, req_uri) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to make evhttp request, errno: " << err << ", err msg: " << strerror(err));
        goto err_out;
    }

    LOG_TRACE("after evhttp_make_request");

    LOG_TRACE("http version, major: " << (int) evhttp_req->major << ", minor: " << (int) evhttp_req->minor
              << ", evhttp conn: " << evhttp_conn << ", flags: " << evhttp_connection_get_flags(evhttp_conn)
              << ", evhttp req: " << evhttp_req << ", flags: " << evhttp_req->flags
              << ", callback_arg: " << callback_arg);

    if (need_encode && encoded_uri != NULL)
    {
        free(encoded_uri);
    }

    callback_arg_set_.insert(callback_arg);
    return 0;

err_out:
    if (need_encode && (encoded_uri != NULL))
    {
        free(encoded_uri);
    }

    if (callback_arg != NULL)
    {
        callback_arg->Release();
    }

    if (evhttp_req != NULL)
    {
        evhttp_request_free(evhttp_req);
    }

    return -1;
}
}
