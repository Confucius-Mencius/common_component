#include "http_client.h"

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
#include <event2/bufferevent_ssl.h>
#include <openssl/err.h>
#endif

#include "log_util.h"
#include "str_util.h"

namespace http
{
void Client::HTTPConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg)
{
    Client* client = static_cast<Client*>(arg);
    LOG_TRACE("Client::HTTPConnClosedCallback, evhttp conn: " << evhttp_conn
              << ", client: " << client << ", " << client->peer_);
}

void Client::HTTPSConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg)
{
    Client* client = static_cast<Client*>(arg);
    LOG_TRACE("Client::HTTPConnClosedCallback, evhttp conn: " << evhttp_conn
              << ", client: " << client << ", " << client->peer_);
}

void Client::HTTPReqDoneCallback(struct evhttp_request* evhttp_req, void* arg)
{
    const int err = EVUTIL_SOCKET_ERROR();
    CallbackArg* callback_arg = static_cast<CallbackArg*>(arg);
    LOG_TRACE("Client::HTTPReqDoneCallback, client: " << callback_arg->http_client << ", "
              << callback_arg->http_client->peer_);

    if (nullptr == evhttp_req)
    {
        // 对端在处理req时挂掉,则会走到这里；对端关闭连接也会走到这里
        LOG_TRACE("evhttp req is null");

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
        if (callback_arg->https)
        {
            unsigned long osl_err = 0;
            bool printed_err = false;
            while ((osl_err = bufferevent_get_openssl_error(callback_arg->buf_event)))
            {
                char buf[128] = "";
                ERR_error_string_n(osl_err, buf, sizeof(buf));
                LOG_ERROR("Client::HTTPReqDoneCallback, https, " << buf << ", " << callback_arg->http_client->peer_);
                printed_err = true;
            }

            if (!printed_err && err != 0)
            {
                LOG_ERROR("socket error, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err) << ", "
                          << callback_arg->http_client->peer_);
            }
        }
#endif

        const TransID& trans_id = callback_arg->trans_id;
        TransCtx* trans_ctx = callback_arg->http_client->client_center_ctx_->trans_center->GetTransCtx(trans_id);
        if (nullptr == trans_ctx)
        {
            LOG_WARN("failed to get trans ctx by trans id: " << trans_id << ", maybe canceled, "
                     << callback_arg->http_client->peer_);
        }
        else
        {
            if (trans_ctx->sink != nullptr)
            {
                trans_ctx->sink->OnClosed(trans_id, callback_arg->http_client->GetPeer(), trans_ctx->data, trans_ctx->len);
            }

            callback_arg->http_client->client_center_ctx_->trans_center->CancelTrans(trans_id);
        }
    }
    else
    {
        callback_arg->http_client->OnHTTPReqDone(callback_arg->trans_id, callback_arg->http_client->GetPeer(),
                callback_arg->https, evhttp_req);
    }

    callback_arg->http_client->callback_arg_set_.erase(callback_arg);
    callback_arg->Release();

    // 看libevent源码，回调完后会自动释放evhttp_req。
}

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
void Client::HTTPReqErrorCallback(evhttp_request_error err, void* arg)
{
    LOG_TRACE("Client::HTTPReqErrorCallback, err: " << err << ", arg: " << arg);
}
#endif

Client::Client() : peer_(), callback_arg_set_()
{
    client_center_ = nullptr;
    client_center_ctx_ = nullptr;
    ssl_ctx_ = nullptr;
    ssl_ = nullptr;
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
    if (nullptr == ctx)
    {
        return -1;
    }

    client_center_ctx_ = (ClientCenterCtx*) ctx;

    /* An OpenSSL context holds data that new SSL connections will
     * be created from. */
    ssl_ctx_ = SSL_CTX_new(SSLv23_client_method());
    if (nullptr == ssl_ctx_)
    {
        const int err = errno;
        LOG_ERROR("SSL_CTX_new failed, errno: " << err << ", err msg: " << strerror(err));;
        return -1;
    }

    /* Find the certificate authority (which we will use to
     * validate the server) and add it to the context. */
//    SSL_CTX_load_verify_locations(sctx, "certificate-authorities.pem", nullptr); // TODO 这个应该是客户端的校验，这里先注释掉

    SSL_CTX_set_verify(ssl_ctx_, SSL_VERIFY_NONE, nullptr); // 如果为SSL_VERIFY_PEER表示客户端会做校验

    /* Create a new SSL connection from our SSL context */
    ssl_ = SSL_new(ssl_ctx_);
    if (nullptr == ssl_)
    {
        const int err = errno;
        LOG_ERROR("SSL_new failed, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    return 0;
}

void Client::Finalize()
{
    for (CallbackArgSet::iterator it = callback_arg_set_.begin(); it != callback_arg_set_.end(); ++it)
    {
        (*it)->Release();
    }

    callback_arg_set_.clear();

    if (ssl_ctx_ != nullptr)
    {
        SSL_CTX_free(ssl_ctx_);
        ssl_ctx_ = nullptr;
    }
}

int Client::Activate()
{
    return 0;
}

void Client::Freeze()
{
}

TransID Client::Get(const GetParams& params, const AsyncCtx* async_ctx)
{
    TransCtx trans_ctx;
    trans_ctx.peer = peer_;
    trans_ctx.timeout_sec = async_ctx->timeout_sec;
    trans_ctx.passback = 0;

    if (async_ctx != nullptr)
    {
        trans_ctx.sink = async_ctx->sink;
        trans_ctx.data = (char*) async_ctx->data;
        trans_ctx.len = async_ctx->len;
    }

    TransID trans_id = client_center_ctx_->trans_center->RecordTransCtx(&trans_ctx);
    if (INVALID_TRANS_ID == trans_id)
    {
        return trans_id;
    }

    // 这里没有用total_retry参数，使用http自己的max_retry机制
    if (DoHTTPReq(trans_id, params.uri, params.uri_len, params.need_encode, params.headers, nullptr, 0, params.https) != 0)
    {
        client_center_ctx_->trans_center->CancelTrans(trans_id);
        return INVALID_TRANS_ID;
    }

    return trans_id;
}

TransID Client::Post(const PostParams& params, const AsyncCtx* async_ctx)
{
    TransCtx trans_ctx;
    trans_ctx.peer = peer_;
    trans_ctx.timeout_sec = async_ctx->timeout_sec;
    trans_ctx.passback = 0;

    if (async_ctx != nullptr)
    {
        trans_ctx.sink = async_ctx->sink;
        trans_ctx.data = (char*) async_ctx->data;
        trans_ctx.len = async_ctx->len;
    }

    TransID trans_id = client_center_ctx_->trans_center->RecordTransCtx(&trans_ctx);
    if (INVALID_TRANS_ID == trans_id)
    {
        return trans_id;
    }

    // 这里没有用total_retry参数，使用http自己的max_retry机制
    if (DoHTTPReq(trans_id, params.get_params.uri, params.get_params.uri_len, params.get_params.need_encode,
                  params.get_params.headers, params.data, params.len, params.get_params.https) != 0)
    {
        client_center_ctx_->trans_center->CancelTrans(trans_id);
        return INVALID_TRANS_ID;
    }

    return trans_id;
}

void Client::OnHTTPReqDone(TransID trans_id, const Peer& peer, bool https, struct evhttp_request* evhttp_req)
{
    LOG_TRACE("Client::OnHTTPReqDone, trans id: " << trans_id << ", https: " << https
              << ", evhttp req: " << evhttp_req);

    const int status_code = evhttp_request_get_response_code(evhttp_req);
    LOG_TRACE("http code: " << status_code);

    TransCtx* trans_ctx = client_center_ctx_->trans_center->GetTransCtx(trans_id);
    if (nullptr == trans_ctx)
    {
        LOG_WARN("failed to get trans ctx by trans id: " << trans_id << ", maybe canceled");
        return;
    }

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    if (EVREQ_HTTP_TIMEOUT == status_code)
#else
    if (0 == status_code)
#endif
    {
        if (trans_ctx->sink != nullptr)
        {
            trans_ctx->sink->OnTimeout(trans_id, peer, trans_ctx->data, trans_ctx->len);
        }

        client_center_ctx_->trans_center->CancelTrans(trans_id);
        return;
    }

    char* status_line = nullptr; // req->response_code_line;
    (void) status_line;

    HeaderMap headers;

    struct evkeyvalq* input_headers = evhttp_request_get_input_headers(evhttp_req);
    if (input_headers != nullptr)
    {
        for (struct evkeyval* header = input_headers->tqh_first; header != nullptr; header = header->next.tqe_next)
        {
            headers.insert(
                HeaderMap::value_type(header->key, header->value));
        }
    }

    struct evbuffer* input_buf = evhttp_request_get_input_buffer(evhttp_req);
    const size_t rsp_body_len = evbuffer_get_length(input_buf);
    char* rsp_body = nullptr;

    if (rsp_body_len > 0)
    {
        rsp_body = new char[rsp_body_len + 1];
        evbuffer_remove(input_buf, rsp_body, rsp_body_len);
        rsp_body[rsp_body_len] = '\0';
    }

    if (trans_ctx->sink != nullptr)
    {
        Rsp http_rsp;
        http_rsp.https = https;
        http_rsp.status_code = status_code;
        http_rsp.status_line = status_line;
        http_rsp.headers = &headers;
        http_rsp.rsp_body = rsp_body;
        http_rsp.rsp_body_len = rsp_body_len;

        trans_ctx->sink->OnRecvHTTPRsp(trans_id, peer, &http_rsp, trans_ctx->data, trans_ctx->len);
    }

    client_center_ctx_->trans_center->CancelTrans(trans_id);

    if (rsp_body != nullptr)
    {
        delete[] rsp_body;
    }
}

struct evhttp_connection* Client::CreateHTTPConn()
{
    // header size和body size的大小均不限制，重连间隔初始为2秒，以后每次都翻倍。
    struct evhttp_connection* evhttp_conn = evhttp_connection_base_new(
            client_center_ctx_->thread_ev_base, nullptr, peer_.addr.c_str(), peer_.port);
    if (nullptr == evhttp_conn)
    {
        const int err = errno;
        LOG_ERROR("failed to create evhttp conn to " << peer_ << ", errno: " << err << ", err msg: " << strerror(err));
        return nullptr;
    }

    if (client_center_ctx_->http_conn_max_retry > 0 || -1 == client_center_ctx_->http_conn_max_retry)
    {
        LOG_DEBUG("http conn max retry: " << client_center_ctx_->http_conn_max_retry);
        evhttp_connection_set_retries(evhttp_conn, client_center_ctx_->http_conn_max_retry); // -1 repeats indefinitely

        struct timeval tv = { 1, 0 };
        evhttp_connection_set_initial_retry_tv(evhttp_conn, &tv);
    }

    if (client_center_ctx_->http_conn_timeout > 0)
    {
        LOG_DEBUG("http conn timeout: " << client_center_ctx_->http_conn_timeout);
        // timeout跟keep alive相关，在这段时间内如果没有消息传递，则关闭连接 TODO 待验证
        evhttp_connection_set_timeout(evhttp_conn, client_center_ctx_->http_conn_timeout);
    }

    // 观察libevent的处理流程，发现一段时间后该连接上没有数据传输则会进到回调中。回调中不用做任何处理，下次请求仍可以复用该连接。
    evhttp_connection_set_closecb(evhttp_conn, Client::HTTPConnClosedCallback, this);

    /* Tell libevent to free the connection when the request finishes */
    evhttp_connection_free_on_completion(evhttp_conn);

    return evhttp_conn;
}

struct evhttp_connection* Client::CreateHTTPSConn()
{
#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    /* Now wrap the SSL connection in an SSL bufferevent */
    struct bufferevent* buf_event = bufferevent_openssl_socket_new(
                                        client_center_ctx_->thread_ev_base, -1, ssl_,
                                        BUFFEREVENT_SSL_CONNECTING,
                                        BEV_OPT_CLOSE_ON_FREE | BEV_OPT_DEFER_CALLBACKS);
    if (nullptr == buf_event)
    {
        const int err = errno;
        LOG_ERROR("failed to create buffer event openssl socket, errno: " << err << ", err msg: " << strerror(err));
        return nullptr;
    }

    bufferevent_openssl_set_allow_dirty_shutdown(buf_event, 1);

    /* Newly-added function in libevent 2.1 which allows us to specify
     * our own bufferevent (e. g. one with SSL) when creating a new
     * HTTP connection.  Sorry, not available in libevent 2.0. */
    // header size和body size的大小均不限制，重连间隔初始为2秒，以后每次都翻倍。
    struct evhttp_connection* evhttp_conn = evhttp_connection_base_bufferevent_new(client_center_ctx_->thread_ev_base, nullptr, buf_event,
                                            peer_.addr.c_str(), peer_.port);
    if (nullptr == evhttp_conn)
    {
        const int err = errno;
        LOG_ERROR("failed to create evhttps conn to " << peer_ << ", errno: " << err << ", err msg: " << strerror(err));
        bufferevent_free(buf_event);
        return nullptr;
    }

    if (client_center_ctx_->http_conn_max_retry > 0)
    {
        LOG_DEBUG("https conn max retry: " << client_center_ctx_->http_conn_max_retry);
        evhttp_connection_set_retries(evhttp_conn, client_center_ctx_->http_conn_max_retry);

        struct timeval tv = { 1, 0 };
        evhttp_connection_set_initial_retry_tv(evhttp_conn, &tv);
    }

    if (client_center_ctx_->http_conn_timeout > 0)
    {
        LOG_DEBUG("https conn timeout: " << client_center_ctx_->http_conn_timeout);
        // timeout跟keep alive相关，在这段时间内如果没有消息传递，则关闭连接 TODO 待验证
        evhttp_connection_set_timeout(evhttp_conn, client_center_ctx_->http_conn_timeout);
    }

    // 观察libevent的处理流程，发现一段时间后该连接上没有数据传输则会进到回调中。回调中不用做任何处理，下次请求仍可以复用该连接。
    evhttp_connection_set_closecb(evhttp_conn, Client::HTTPSConnClosedCallback, this);

    /* Tell libevent to free the connection when the request finishes */
    // 看libevent源码，evhttp_connection_free中会释放buf_event_
    evhttp_connection_free_on_completion(evhttp_conn);

    return evhttp_conn;
#else
    return nullptr;
#endif
}

int Client::DoHTTPReq(TransID trans_id, const char* uri, int uri_len, bool need_encode, const HeaderMap* headers,
                      const void* data, size_t data_len, bool https)
{
    if (nullptr == uri)
    {
        return -1;
    }

    LOG_TRACE("trans id: " << trans_id << ", " << peer_ << ", uri: " << uri << ", uri len: " << uri_len
              << ", need encode: " << need_encode << ", data len: " << data_len);

    char* req_uri = (char*) uri;
    char* encoded_uri = nullptr;
    CallbackArg* callback_arg = nullptr;
    struct evhttp_request* evhttp_req = nullptr;
    struct evkeyvalq* output_headers = nullptr;
    evhttp_cmd_type cmd_type = EVHTTP_REQ_GET;

    struct evhttp_connection* evhttp_conn = (https ? CreateHTTPSConn() : CreateHTTPConn());
    if (nullptr == evhttp_conn)
    {
        return -1;
    }

    LOG_DEBUG("evhttp conn: " << evhttp_conn);

    std::stringstream host_stream;
    host_stream.str("");
    host_stream << peer_.addr << ":" << peer_.port;

    if (need_encode)
    {
        encoded_uri = evhttp_uriencode(uri, uri_len, 0);
        if (nullptr == encoded_uri)
        {
            LOG_ERROR("failed to encode uri: " << uri << ", len: " << uri_len);
            return -1;
        }

        req_uri = encoded_uri;
    }

    callback_arg = CallbackArg::Create();
    if (nullptr == callback_arg)
    {
        const int err = errno;
        LOG_ERROR("failed to create callback arg, errno: " << err << ", err msg: " << strerror(err));
        goto err_out;
    }

    callback_arg->http_client = this;
    callback_arg->https = https;
    callback_arg->buf_event = evhttp_connection_get_bufferevent(evhttp_conn);
    callback_arg->trans_id = trans_id;

    evhttp_req = evhttp_request_new(Client::HTTPReqDoneCallback, callback_arg); // 处理完成后libevent会自动释放req
    if (nullptr == evhttp_req)
    {
        const int err = errno;
        LOG_ERROR("failed to create evhttp req, errno: " << err << ", err msg: " << strerror(err));
        goto err_out;
    }

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    // 注意：注册这个回调纯粹为了观察libevent的处理流程，无实际意义
    evhttp_request_set_error_cb(evhttp_req, Client::HTTPReqErrorCallback);
#endif

    // http headers
    output_headers = evhttp_request_get_output_headers(evhttp_req);

    if (headers != nullptr)
    {
        for (HeaderMap::const_iterator it = headers->begin(); it != headers->end(); ++it)
        {
            if (0 == strcasecmp(it->first.c_str(), "host"))
            {
                continue;
            }

            evhttp_add_header(output_headers, it->first.c_str(), it->second.c_str());
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

    for (struct evkeyval* header = output_headers->tqh_first; header != nullptr; header = header->next.tqe_next)
    {
        LOG_DEBUG(header->key << ": " << header->value);
    }

    // TODO Connection: close和Connection: keep-alive请求头和回复头的处理流程

    // http body
    if (data != nullptr && data_len > 0)
    {
        // 有些web服务器需要根据Content-Type对post内容做解析，所以这里必须加上Content-Type
        if (nullptr == evhttp_find_header(output_headers, "Content-Type"))
        {
            evhttp_add_header(output_headers, "Content-Type", "application/x-www-form-urlencoded"); // TODO 默认的content type怎么设？
        }

        cmd_type = EVHTTP_REQ_POST;

        struct evbuffer* output_buf = evhttp_request_get_output_buffer(evhttp_req);
        if (nullptr == output_buf)
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

    if (evhttp_make_request(evhttp_conn, evhttp_req, cmd_type, req_uri) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to make evhttp request, errno: " << err << ", err msg: " << strerror(err));
        goto err_out;
    }

    LOG_DEBUG("http version major: " << (int) evhttp_req->major << ", minor: " << (int) evhttp_req->minor
              << ", evhttp conn: " << evhttp_conn
              << ", evhttp req: " << evhttp_req << ", flags: " << evhttp_req->flags
              << ", callback_arg: " << callback_arg);

    if (need_encode && encoded_uri != nullptr)
    {
        free(encoded_uri);
    }

    callback_arg_set_.insert(callback_arg);
    return 0;

err_out:
    if (need_encode && (encoded_uri != nullptr))
    {
        free(encoded_uri);
    }

    if (callback_arg != nullptr)
    {
        callback_arg->Release();
    }

    if (evhttp_req != nullptr)
    {
        evhttp_request_free(evhttp_req);
    }

    return -1;
}
}
