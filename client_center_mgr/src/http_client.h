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
    CREATE_FUNC(Client);

    static void HttpConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg);
    static void HttpsConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg);
    static void HttpReqDoneCallback(struct evhttp_request* evhttp_req, void* arg);

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    static void HttpReqErrorCallback(evhttp_request_error err, void* arg);
#endif

//    static void OnConnCleanupEvent(int sock_fd, short which, void* arg);

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

    TransId Head(const HeadParams& params, const base::AsyncCtx* async_ctx) override;
    TransId Get(const GetParams& params, const base::AsyncCtx* async_ctx) override;
    TransId Post(const PostParams& params, const base::AsyncCtx* async_ctx) override;

public:
    void SetClientCenter(ClientCenter* client_center)
    {
        client_center_ = client_center;
    }

    void SetPeer(const Peer& peer)
    {
        peer_ = peer;
    }

    void OnHttpReqDone(TransId trans_id, const Peer& peer, bool https, struct evhttp_request* evhttp_req);

private:
    int CreateEvHttpConn(const Peer& peer);

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    int CreateEvHttpsConn(const Peer& peer);
#endif

    /**
     * encoded_uri  形如："/index.php?id=1"
     */
    int DoHttpReq(TransId trans_id, const char* uri, int uri_len, bool need_encode, const KeyValMap* header_map,
                  const void* data, size_t data_len, bool https);

private:
    ClientCenter* client_center_;
    const ClientCenterCtx* client_center_ctx_;
    Peer peer_;
    struct evhttp_connection* evhttp_conn_;

    struct CallbackArg
    {
        Client* http_client;
        bool https;
        TransId trans_id;
//        struct event* cleanup_event;

        CallbackArg()
        {
            http_client = NULL;
            https = false;
            trans_id = INVALID_TRANS_ID;
//            cleanup_event = NULL;
        }

        CREATE_FUNC(CallbackArg);

        void Release()
        {
//            if (cleanup_event != NULL)
//            {
//                event_del(cleanup_event);
//                event_free(cleanup_event);
//                cleanup_event = NULL;
//            }

            delete this;
        }
    };

    typedef std::set<CallbackArg*> CallbackArgSet;
    CallbackArgSet callback_arg_set_;

    // https conn
    SSL_CTX* sctx_;
    struct bufferevent* buf_event_;
    struct evhttp_connection* evhttps_conn_;
};
}

#endif // CLIENT_CENTER_MGR_SRC_HTTP_CLIENT_H_
