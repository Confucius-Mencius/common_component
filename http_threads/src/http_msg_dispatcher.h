#ifndef HTTP_THREADS_SRC_HTTP_MSG_DISPATCHER_H_
#define HTTP_THREADS_SRC_HTTP_MSG_DISPATCHER_H_

#include <map>
#include "embed_http_msg_handler_mgr.h"

namespace http
{
class ThreadSink;

class MsgDispatcher : public MsgDispatcherInterface
{
public:
    MsgDispatcher();
    virtual ~MsgDispatcher();

    static void ReqCallback(struct evhttp_request* evhttp_req, void* arg, bool https);
    static void HttpReqCallback(struct evhttp_request* evhttp_req, void* arg);
    static void HttpsReqCallback(struct evhttp_request* evhttp_req, void* arg);
    static void GenericCallback(struct evhttp_request* evhttp_req, void* arg, bool https);
    static void HttpGenericCallback(struct evhttp_request* evhttp_req, void* arg);
    static void HttpsGenericCallback(struct evhttp_request* evhttp_req, void* arg);
    static void ConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg, bool https);
    static void HttpConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg);
    static void HttpsConnClosedCallback(struct evhttp_connection* evhttp_conn, void* arg);

    void OnHttpReq(struct evhttp_request* evhttp_req, MsgHandlerInterface* http_msg_handler, bool https);
    void OnHttpConnClosed(struct evhttp_connection* evhttp_conn, bool https);
    void HandleHttpReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                       MsgHandlerInterface* http_msg_handler);
    void HandleHttpPostReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                           const KeyValMap* http_header_map, const KeyValMap* http_query_map,
                           MsgHandlerInterface* http_msg_handler);
    void HandleHttpPutReq(const ConnGuid* conn_guid, struct evhttp_request* evhttp_req, bool https,
                          const KeyValMap* http_header_map, const KeyValMap* http_query_map,
                          MsgHandlerInterface* http_msg_handler);

    ////////////////////////////////////////////////////////////////////////////////
    int Initialize(const void* ctx);
    void Finalize();
    int Activate();
    void Freeze();

    void PrintParsedInfo(struct evhttp_request* req);
    int GetPeerAddr(std::string& addr, unsigned short& port, struct evhttp_request* evhttp_req);
    int ParseUri(std::string& path, std::string& query, const char* uri, bool need_decode);

    ///////////////////////// MsgDispatcherInterface /////////////////////////
    virtual int AttachMsgHandler(const char* path, MsgHandlerInterface* http_msg_handler);
    virtual void DetachMsgHandler(const char* path);
    virtual int AttachGeneralMsgHandler(MsgHandlerInterface* general_req_handler);
    virtual void DetachGeneralMsgHandler();

public:
    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

private:
    ThreadSink* thread_sink_;

    struct CallbackArgs
    {
        MsgDispatcher* msg_dispatcher;
        MsgHandlerInterface* msg_handler;
    };

    typedef std::map<std::string, CallbackArgs> MsgHandlerMap; // path ->
    MsgHandlerMap msg_handler_map_;

    LogicCtx logic_ctx_;
    EmbedMsgHandlerMgr embed_msg_handler_mgr_;
};
}

#endif // HTTP_THREADS_SRC_HTTP_MSG_DISPATCHER_H_
