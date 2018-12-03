#ifndef HTTP_THREADS_SRC_HTTP_THREAD_SINK_H_
#define HTTP_THREADS_SRC_HTTP_THREAD_SINK_H_

// 2.1.5-beta 0x2010500 2.1.5-beta 0x2010500  -- 该版本支持https
// 2.0.22-stable 0x2001600 2.0.22-stable 0x2001600  -- 该版本不支持https

#include <evhttp.h>
#include <openssl/ssl.h>
#include "client_center_mgr_interface.h"
#include "conn_center_mgr_interface.h"
#include "http_logic_interface.h"
#include "http_msg_dispatcher.h"
#include "http_scheduler.h"
#include "http_threads_interface.h"
#include "mem_util.h"
#include "module_loader.h"
#include "thread_center_interface.h"

namespace http
{
struct LogicItem
{
    std::string logic_so_path;
    ModuleLoader logic_loader;
    LogicInterface* logic;

    LogicItem() : logic_so_path(), logic_loader()
    {
        logic = NULL;
    }
};

typedef std::vector<LogicItem> LogicItemVec;

class ThreadSink : public ThreadSinkInterface, public tcp::NfySinkInterface
{
    CREATE_FUNC(ThreadSink);
#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    /**
     * This callback is responsible for creating a new SSL connection
     * and wrapping it in an OpenSSL bufferevent.  This is the way
     * we implement an https server instead of a plain old http server.
     */
    static struct bufferevent* HttpsBevCallback(struct event_base* ev_base, void* arg);
#endif

public:
    ThreadSink();
    virtual ~ThreadSink();

    ///////////////////////// ThreadSinkInterface /////////////////////////
    void Release() override;
    int OnInitialize(ThreadInterface* thread) override;
    void OnFinalize() override;
    int OnActivate() override;
    void OnFreeze() override;
    void OnThreadStartOk() override;
    void OnStop() override;
    void OnReload() override;
    void OnTask(const Task* task) override;
    bool CanExit() const override;

    ///////////////////////// tcp::NfySinkInterface /////////////////////////
    void OnRecvNfy(const Peer& peer, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len) override;

public:
    void SetThreadsCtx(const ThreadsCtx* threads_ctx)
    {
        threads_ctx_ = threads_ctx;
    }

    void SetHttpThreadGroup(ThreadGroupInterface* http_thread_group)
    {
        http_thread_group_ = http_thread_group;
    }

    ConnCenterInterface* GetConnCenter()
    {
        return conn_center_;
    }

    tcp::ClientCenterInterface* GetTcpClientCenter()
    {
        return tcp_client_center_;
    }

    http::ClientCenterInterface* GetHttpClientCenter()
    {
        return http_client_center_;
    }

    udp::ClientCenterInterface* GetUdpClientCenter()
    {
        return udp_client_center_;
    }

    ThreadGroupInterface* GetHttpThreadGroup()
    {
        return http_thread_group_;
    }

    void SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group);

    void SetListenSocketFd(int http_listen_sock_fd, int https_listen_sock_fd)
    {
        http_listen_sock_fd_ = http_listen_sock_fd;

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
        https_listen_sock_fd_ = https_listen_sock_fd;
#endif
    }

    app_frame::ConfMgrInterface* GetConfMgr() const
    {
        return threads_ctx_->conf_mgr;
    }

    Scheduler* GetScheduler() const
    {
        return const_cast<Scheduler*>(&scheduler_);
    }

    struct evhttp* GetHttpServer()
    {
        return http_server_;
    }

    struct evhttp* GetHttpsServer()
    {
#if LIBEVENT_VERSION_NUMBER >= 0x2010500
        return https_server_;
#else
        return NULL;
#endif
    }

private:
    int InitHttpServer();
    int InitHttpsServer();
    int SetEvhttpOpts(struct evhttp* http_server);
    int SetupHttpsCerts(SSL_CTX* ctx, const char* certificate_chain_file_path, const char* private_key_file_path);
    int LoadLocalLogic();
    int LoadLogicGroup();
    void OnTransReturn(ThreadInterface* source_thread, const ConnGuid* conn_guid, const MsgHead& msg_head,
                       const void* msg_body, size_t msg_body_len);

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* http_thread_group_;

    ConnCenterInterface* conn_center_;

    tcp::ClientCenterInterface* tcp_client_center_;
    http::ClientCenterInterface* http_client_center_;
    udp::ClientCenterInterface* udp_client_center_;

    Scheduler scheduler_;

    ModuleLoader local_logic_loader_;
    LocalLogicInterface* local_logic_;

    LogicItemVec logic_item_vec_;

    RelatedThreadGroup* related_thread_group_;

    MsgDispatcher http_msg_dispatcher_;

    int http_listen_sock_fd_;
    struct evhttp* http_server_;

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    SSL_CTX* ssl_ctx_;
    EC_KEY* ecdh_;
    int https_listen_sock_fd_;
    struct evhttp* https_server_;
#endif
};
}

#endif // HTTP_THREADS_SRC_HTTP_THREAD_SINK_H_
