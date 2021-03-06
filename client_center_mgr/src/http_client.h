#ifndef CLIENT_CENTER_MGR_SRC_HTTP_CLIENT_H_
#define CLIENT_CENTER_MGR_SRC_HTTP_CLIENT_H_

#include <set>
#include <evhttp.h>
#include <openssl/ssl.h>
#include "http_client_center_interface.h"
#include "mem_util.h"
#include "trans_center_interface.h"

namespace http
{
class ClientCenter;

class Client : public ModuleInterface, public ClientInterface
{
    CREATE_FUNC(Client)

    static void HTTPConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg);
    static void HTTPSConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg);
    static void HTTPReqDoneCallback(struct evhttp_request* evhttp_req, void* arg);

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    static void HTTPReqErrorCallback(evhttp_request_error err, void* arg);
#endif

    static void OnConnCleanupEvent(int sock_fd, short which, void* arg);

public:
    Client();
    virtual ~Client();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ClientInterface /////////////////////////
    const Peer& GetPeer() const override
    {
        return peer_;
    }

    TransID Get(const GetParams& params, const AsyncCtx* async_ctx) override;
    TransID Post(const PostParams& params, const AsyncCtx* async_ctx) override;

public:
    void SetClientCenter(ClientCenter* client_center)
    {
        client_center_ = client_center;
    }

    void SetPeer(const Peer& peer)
    {
        peer_ = peer;
    }

    void OnHTTPReqDone(TransID trans_id, const Peer& peer, bool https, struct evhttp_request* evhttp_req);

private:
    struct evhttp_connection* CreateHTTPConn();
    struct evhttp_connection* CreateHTTPSConn();

    /**
     * encoded_uri 形如："/index.php?id=1"
     */
    int DoHTTPReq(TransID trans_id, const char* uri, int uri_len, bool need_encode, const HeaderMap* headers,
                  const void* data, size_t len, bool https);

private:
    ClientCenter* client_center_;
    const ClientCenterCtx* client_center_ctx_;
    Peer peer_;

    struct CallbackArg
    {
        Client* http_client;
        bool https;
        struct evhttp_connection* evhttp_conn;
        TransID trans_id;
        struct event* cleanup_event;

        CallbackArg()
        {
            http_client = nullptr;
            https = false;
            evhttp_conn = nullptr;
            trans_id = INVALID_TRANS_ID;
            cleanup_event = nullptr;
        }

        CREATE_FUNC(CallbackArg)

        void Release()
        {
            if (evhttp_conn != nullptr)
            {
                evhttp_connection_free(evhttp_conn);
                evhttp_conn = nullptr;
            }

            if (cleanup_event != nullptr)
            {
                event_del(cleanup_event);
                event_free(cleanup_event);
                cleanup_event = nullptr;
            }

            delete this;
        }
    };

    typedef std::set<CallbackArg*> CallbackArgSet;
    CallbackArgSet callback_arg_set_;

    // https
    SSL_CTX* ssl_ctx_;
    SSL* ssl_;
};
}

#endif // CLIENT_CENTER_MGR_SRC_HTTP_CLIENT_H_
