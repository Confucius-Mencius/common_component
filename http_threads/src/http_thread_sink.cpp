#include "http_thread_sink.h"
#include <event2/bufferevent_ssl.h>
#include <openssl/err.h>

#if defined(NDEBUG)
#include <gperftools/profiler.h>
#endif

#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "msg_codec_center_interface.h"
#include "msg_handler_interface.h"
#include "str_util.h"
#include "trans_center_interface.h"

namespace http
{
#if LIBEVENT_VERSION_NUMBER >= 0x2010500
struct bufferevent* ThreadSink::HttpsBevCallback(struct event_base* ev_base, void* arg)
{
    SSL_CTX* ctx = (SSL_CTX*) arg;
    struct bufferevent* r = bufferevent_openssl_socket_new(ev_base, -1, SSL_new(ctx),
                            BUFFEREVENT_SSL_ACCEPTING, BEV_OPT_CLOSE_ON_FREE);
    return r;
}
#endif

ThreadSink::ThreadSink() : scheduler_(), local_logic_loader_(), logic_item_vec_(), http_msg_dispatcher_()
{
    threads_ctx_ = NULL;
    http_thread_group_ = NULL;
    conn_center_ = NULL;
    tcp_client_center_ = NULL;
    http_client_center_ = NULL;
    udp_client_center_ = NULL;
    local_logic_ = NULL;
    related_thread_group_ = NULL;

    http_listen_sock_fd_ = -1;
    http_server_ = NULL;
#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    ssl_ctx_ = NULL;
    ecdh_ = NULL;
    https_listen_sock_fd_ = -1;
    https_server_ = NULL;
#endif
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

    SAFE_RELEASE_MODULE(local_logic_, local_logic_loader_);
    delete this;
}

int ThreadSink::OnInitialize(ThreadInterface* thread)
{
    if (ThreadSinkInterface::OnInitialize(thread) != 0)
    {
        return -1;
    }

    // http
    if (InitHttpServer() != 0)
    {
        return -1;
    }

    // https
    if (InitHttpsServer() != 0)
    {
        return -1;
    }

    ConnCenterCtx http_conn_center_ctx;
    http_conn_center_ctx.timer_axis = thread_->GetTimerAxis();
    http_conn_center_ctx.content_type = threads_ctx_->conf_mgr->GetHttpContentType();
    http_conn_center_ctx.no_cache = threads_ctx_->conf_mgr->HttpNoCache();

    conn_center_ = thread_->GetConnCenterMgr()->CreateHttpConnCenter(&http_conn_center_ctx);
    if (NULL == conn_center_)
    {
        return -1;
    }

    tcp::ClientCenterCtx tcp_client_center_ctx;
    tcp_client_center_ctx.thread_ev_base = thread_->GetThreadEvBase();

    {
        MsgCodecCtx msg_codec_ctx;
        msg_codec_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetTcpMaxMsgBodyLen();
        msg_codec_ctx.do_checksum = threads_ctx_->conf_mgr->TcpDoChecksum();

        MsgCodecInterface* msg_codec_ = threads_ctx_->msg_codec_center->CreateMsgCodec(&msg_codec_ctx);
        if (NULL == msg_codec_)
        {
            LOG_ERROR("failed to create tcp msg codec");
            return -1;
        }

        tcp_client_center_ctx.msg_codec = msg_codec_;
    }

    tcp_client_center_ctx.timer_axis = thread_->GetTimerAxis();
    tcp_client_center_ctx.trans_center = thread_->GetTransCenter();
    tcp_client_center_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetTcpMaxMsgBodyLen();
    tcp_client_center_ctx.reconnect_interval = {threads_ctx_->conf_mgr->GetPeerTcpConnIntervalSec(),
                                                threads_ctx_->conf_mgr->GetPeerTcpConnIntervalUsec()
                                               };

    tcp_client_center_ = thread_->GetClientCenterMgr()->CreateTcpClientCenter(&tcp_client_center_ctx);
    if (NULL == tcp_client_center_)
    {
        return -1;
    }

    ClientCenterCtx http_client_center_ctx;
    http_client_center_ctx.thread_ev_base = thread_->GetThreadEvBase();
    http_client_center_ctx.timer_axis = thread_->GetTimerAxis();
    http_client_center_ctx.trans_center = thread_->GetTransCenter();
    http_client_center_ctx.http_conn_timeout = threads_ctx_->conf_mgr->GetPeerHttpConnTimeout();
    http_client_center_ctx.http_conn_max_retry = threads_ctx_->conf_mgr->GetPeerHttpConnMaxRetry();

    http_client_center_ = thread_->GetClientCenterMgr()->CreateHttpClientCenter(&http_client_center_ctx);
    if (NULL == http_client_center_)
    {
        return -1;
    }

    udp::ClientCenterCtx udp_client_center_ctx;
    udp_client_center_ctx.thread_ev_base = thread_->GetThreadEvBase();

    {
        MsgCodecCtx msg_codec_ctx;
        msg_codec_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetUdpMaxMsgBodyLen();
        msg_codec_ctx.do_checksum = threads_ctx_->conf_mgr->UdpDoChecksum();

        MsgCodecInterface* msg_codec_ = threads_ctx_->msg_codec_center->CreateMsgCodec(&msg_codec_ctx);
        if (NULL == msg_codec_)
        {
            LOG_ERROR("failed to create tcp msg codec");
            return -1;
        }

        udp_client_center_ctx.msg_codec = msg_codec_;
    }

    udp_client_center_ctx.timer_axis = thread_->GetTimerAxis();
    udp_client_center_ctx.trans_center = thread_->GetTransCenter();
    udp_client_center_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetUdpMaxMsgBodyLen();

    udp_client_center_ = thread_->GetClientCenterMgr()->CreateUdpClientCenter(&udp_client_center_ctx);
    if (NULL == udp_client_center_)
    {
        return -1;
    }

    scheduler_.SetThreadSink(this);

    if (scheduler_.Initialize(threads_ctx_) != 0)
    {
        return -1;
    }

    http_msg_dispatcher_.SetThreadSink(this);

    if (http_msg_dispatcher_.Initialize(threads_ctx_) != 0)
    {
        return -1;
    }

    if (LoadLocalLogic() != 0)
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

    SAFE_FINALIZE(local_logic_);

    http_msg_dispatcher_.Finalize();
    scheduler_.Finalize();

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    if (ecdh_ != NULL)
    {
        EC_KEY_free(ecdh_);
        ecdh_ = NULL;
    }

    if (ssl_ctx_ != NULL)
    {
        SSL_CTX_free(ssl_ctx_);
        ssl_ctx_ = NULL;
    }

    if (https_server_ != NULL)
    {
        evhttp_free(https_server_);
        https_server_ = NULL;
    }
#endif

    if (http_server_ != NULL)
    {
        evhttp_free(http_server_);
        http_server_ = NULL;
    }

    ThreadSinkInterface::OnFinalize();
}

int ThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    if (http_msg_dispatcher_.Activate() != 0)
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(local_logic_))
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

    SAFE_FREEZE(local_logic_);

    http_msg_dispatcher_.Freeze();
    ERR_remove_thread_state(NULL); // 线程中使用openssl的清理，否则有内存泄露
    ThreadSinkInterface::OnFreeze();
}

void ThreadSink::OnThreadStartOk()
{
    ThreadSinkInterface::OnThreadStartOk();

#if defined(NDEBUG)
    if (threads_ctx_->conf_mgr->EnableCpuProfiling())
    {
        LOG_INFO("enable cpu profiling");
        ProfilerRegisterThread();
    }
#endif

    pthread_mutex_lock(threads_ctx_->frame_threads_mutex);
    ++(*threads_ctx_->frame_threads_count);
    pthread_cond_signal(threads_ctx_->frame_threads_cond);
    pthread_mutex_unlock(threads_ctx_->frame_threads_mutex);
}

void ThreadSink::OnStop()
{
    ThreadSinkInterface::OnStop();

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnStop();
    }

    if (local_logic_ != NULL)
    {
        local_logic_->OnStop();
    }
}

void ThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnReload();
    }

    if (local_logic_ != NULL)
    {
        local_logic_->OnReload();
    }
}

void ThreadSink::OnTask(const Task* task)
{
    ThreadSinkInterface::OnTask(task);
    const TaskCtx* task_ctx = task->GetCtx();

    switch (task_ctx->task_type)
    {
        case TASK_TYPE_GLOBAL_RETURN_TO_HTTP:
        case TASK_TYPE_WORK_RETURN_TO_HTTP:
        {
            OnTransReturn(task_ctx->source_thread, &task_ctx->conn_guid, task_ctx->msg_head, task_ctx->msg_body,
                          task_ctx->msg_body_len);
        }
        break;

        default:
        {
            LOG_ERROR("invalid task type: " << task_ctx->task_type);
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

    if (local_logic_ != NULL)
    {
        can_exit &= (local_logic_->CanExit() ? 1 : 0);
    }

    return (can_exit != 0);
}

void ThreadSink::OnRecvNfy(const Peer& peer, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
{
    // TODO 使用另外一个msg dispatcher处理。
}

void ThreadSink::SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group)
{
    related_thread_group_ = related_thread_group;

    if (related_thread_group_->global_logic != NULL)
    {
        if (local_logic_ != NULL)
        {
            local_logic_->SetGlobalLogic(related_thread_group_->global_logic);
        }

        for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
        {
            LogicItem& logic_item = *it;
            logic_item.logic->SetGlobalLogic(related_thread_group_->global_logic);
        }
    }

    scheduler_.SetRelatedThreadGroup(related_thread_group);
}

int ThreadSink::InitHttpServer()
{
    if (http_listen_sock_fd_ < 0)
    {
        return 0;
    }

    http_server_ = evhttp_new(thread_->GetThreadEvBase());
    if (NULL == http_server_)
    {
        const int err = errno;
        LOG_ERROR("failed to create http server, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (evhttp_accept_socket(http_server_, http_listen_sock_fd_) != 0)
    {
        const int err = errno;
        LOG_ERROR("evhttp_accept_socket failed, errno: " << err << ", err: " << strerror(err));
        return -1;
    }

    if (SetEvhttpOpts(http_server_) != 0)
    {
        return -1;
    }

    return 0;
}

int ThreadSink::InitHttpsServer()
{
#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    if (https_listen_sock_fd_ < 0)
    {
        return 0;
    }

    app_frame::ConfMgrInterface* conf_mgr = threads_ctx_->conf_mgr;

    const std::string https_certificate_chain_file_path = conf_mgr->GetHttpsCertificateChainFilePath();
    if (0 == https_certificate_chain_file_path.length())
    {
        LOG_ERROR("failed to get https_certificate_chain_file_path");
        return -1;
    }

    if (!FileExist(https_certificate_chain_file_path.c_str()))
    {
        LOG_ERROR("file " << https_certificate_chain_file_path << " not exist");
        return -1;
    }

    const std::string https_private_key_file_path = conf_mgr->GetHttpsPrivateKeyFilePath();
    if (0 == https_private_key_file_path.length())
    {
        LOG_ERROR("failed to get https_private_key_file_path");
        return -1;
    }

    if (!FileExist(https_private_key_file_path.c_str()))
    {
        LOG_ERROR("file " << https_private_key_file_path << " not exist");
        return -1;
    }

    https_server_ = evhttp_new(thread_->GetThreadEvBase());
    if (NULL == https_server_)
    {
        const int err = errno;
        LOG_ERROR("failed to create https server, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    ssl_ctx_ = SSL_CTX_new(SSLv23_server_method());
    if (NULL == ssl_ctx_)
    {
        LOG_ERROR("SSL_CTX_new failed");
        return -1;
    }

    SSL_CTX_set_options(ssl_ctx_, SSL_OP_SINGLE_DH_USE | SSL_OP_SINGLE_ECDH_USE | SSL_OP_NO_SSLv2);

    /* Cheesily pick an elliptic curve to use with elliptic curve ciphersuites.
     * We just hardcode a single curve which is reasonably decent.
     * See http://www.mail-archive.com/openssl-dev@openssl.org/msg30957.html */
    ecdh_ = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    if (NULL == ecdh_)
    {
        LOG_ERROR("EC_KEY_new_by_curve_name failed");
        return -1;
    }

    long ret = SSL_CTX_set_tmp_ecdh(ssl_ctx_, ecdh_);
    if (ret != 1)
    {
        LOG_ERROR("EC_KEY_new_by_curve_name failed, ret: " << ret);
        return -1;
    }

    /* Find and set up our server certificate. */
    if (SetupHttpsCerts(ssl_ctx_, https_certificate_chain_file_path.c_str(), https_private_key_file_path.c_str()) != 0)
    {
        return -1;
    }

    if (evhttp_accept_socket(https_server_, https_listen_sock_fd_) != 0)
    {
        const int err = errno;
        LOG_ERROR("evhttp_accept_socket failed, errno: " << err << ", err: " << strerror(err));
        return -1;
    }

    if (SetEvhttpOpts(https_server_) != 0)
    {
        return -1;
    }

    /* This is the magic that lets evhttp use SSL. */
    evhttp_set_bevcb(https_server_, HttpsBevCallback, ssl_ctx_);
#endif

    return 0;
}

int ThreadSink::SetEvhttpOpts(struct evhttp* http_server)
{
    ///////////////////////////////////////////////////////////////////////////////
    app_frame::ConfMgrInterface* conf_mgr = threads_ctx_->conf_mgr;

    const int max_header_size = conf_mgr->GetHttpMaxHeaderSize();
    if (max_header_size > 0)
    {
        evhttp_set_max_headers_size(http_server, max_header_size);
    }

    ////////////////////////////////////////////////////////////////////////////////
    const int max_body_size = conf_mgr->GetHttpMaxBodySize();
    if (max_body_size > 0)
    {
        evhttp_set_max_body_size(http_server, max_body_size);
    }

    ////////////////////////////////////////////////////////////////////////////////
    const int conn_timeout_sec = conf_mgr->GetHttpConnTimeoutSec();
    const int conn_timeout_usec = conf_mgr->GetHttpConnTimeoutUsec();
    if (conn_timeout_sec > 0 || conn_timeout_usec > 0)
    {
        struct timeval conn_timeout;
        conn_timeout.tv_sec = conn_timeout_sec > 0 ? conn_timeout_sec : 0;
        conn_timeout.tv_usec = conn_timeout_usec > 0 ? conn_timeout_usec : 0;

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
        evhttp_set_timeout_tv(http_server, &conn_timeout);
#else
        if (conn_timeout.tv_sec > 0)
        {
            evhttp_set_timeout(http_server, conn_timeout.tv_sec);
        }
#endif
    }

    return 0;
}

int ThreadSink::SetupHttpsCerts(SSL_CTX* ctx, const char* certificate_chain_file_path,
                                const char* private_key_file_path)
{
    LOG_INFO("certificate chain file: " << certificate_chain_file_path
             << ", private key fiel: " << private_key_file_path);

    // 公钥证书
    int ret = SSL_CTX_use_certificate_chain_file(ctx, certificate_chain_file_path); // todo 有内存泄露
    if (ret != 1)
    {
        LOG_ERROR("SSL_CTX_use_certificate_chain_file failed, ret: " << ret);
        return -1;
    }

    // 私钥
    ret = SSL_CTX_use_PrivateKey_file(ctx, private_key_file_path, SSL_FILETYPE_PEM);
    if (ret != 1)
    {
        LOG_ERROR("SSL_CTX_use_PrivateKey_file failed, ret: " << ret);
        return -1;
    }

    // 检查SSL连接所用的私钥与证书是否匹配
    ret = SSL_CTX_check_private_key(ctx);
    if (ret != 1)
    {
        LOG_ERROR("SSL_CTX_check_private_key failed, ret: " << ret);
        return -1;
    }

    return 0;
}

int ThreadSink::LoadLocalLogic()
{
    if (0 == threads_ctx_->conf_mgr->GetHttpLocalLogicSo().length())
    {
        return 0;
    }

    const std::string local_logic_so_path = GetAbsolutePath(threads_ctx_->conf_mgr->GetHttpLocalLogicSo().c_str(),
                                            threads_ctx_->cur_work_dir);
    LOG_INFO("load local logic so " << local_logic_so_path << " begin");

    if (local_logic_loader_.Load(local_logic_so_path.c_str()) != 0)
    {
        LOG_ERROR(
            "failed to load local logic so " << local_logic_so_path << ", " << local_logic_loader_.GetLastErrMsg());
        return -1;
    }

    local_logic_ = (LocalLogicInterface*) local_logic_loader_.GetModuleInterface();
    if (NULL == local_logic_)
    {
        LOG_ERROR("failed to get local logic interface, " << local_logic_loader_.GetLastErrMsg());
        return -1;
    }

    LOG_INFO("load local logic so " << local_logic_so_path << " ...");

    LogicCtx logic_ctx;
    logic_ctx.argc = threads_ctx_->argc;
    logic_ctx.argv = threads_ctx_->argv;
    logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
    logic_ctx.cur_work_dir = threads_ctx_->cur_work_dir;
    logic_ctx.app_name = threads_ctx_->app_name;
    logic_ctx.conf_center = threads_ctx_->conf_center;
    logic_ctx.timer_axis = thread_->GetTimerAxis();
    logic_ctx.time_service = thread_->GetTimeService();
    logic_ctx.random_engine = thread_->GetRandomEngine();
    logic_ctx.conn_center = conn_center_;
    logic_ctx.msg_dispatcher = NULL;
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.local_logic = local_logic_;
    logic_ctx.thread_ev_base = thread_->GetThreadEvBase();

    if (local_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_INFO("load local logic so " << local_logic_so_path << " end");
    return 0;
}

int ThreadSink::LoadLogicGroup()
{
    // logic group
    LogicItem logic_item;
    logic_item.logic = NULL;

    const StrGroup logic_so_group = threads_ctx_->conf_mgr->GetHttpLogicSoGroup();

    for (StrGroup::const_iterator it = logic_so_group.begin(); it != logic_so_group.end(); ++it)
    {
        logic_item.logic_so_path = GetAbsolutePath((*it).c_str(), threads_ctx_->cur_work_dir);
        logic_item_vec_.push_back(logic_item);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        LogicItem& logic_item = *it;
        LOG_INFO("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so " << logic_item.logic_so_path << ", "
                      << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = (LogicInterface*) logic_item.logic_loader.GetModuleInterface();
        if (NULL == logic_item.logic)
        {
            LOG_ERROR("failed to get logic interface, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        LOG_INFO("load logic so " << logic_item.logic_so_path << " ...");

        LogicCtx logic_ctx;
        logic_ctx.argc = threads_ctx_->argc;
        logic_ctx.argv = threads_ctx_->argv;
        logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
        logic_ctx.cur_work_dir = threads_ctx_->cur_work_dir;
        logic_ctx.app_name = threads_ctx_->app_name;
        logic_ctx.conf_center = threads_ctx_->conf_center;
        logic_ctx.timer_axis = thread_->GetTimerAxis();
        logic_ctx.time_service = thread_->GetTimeService();
        logic_ctx.random_engine = thread_->GetRandomEngine();
        logic_ctx.conn_center = conn_center_;
        logic_ctx.msg_dispatcher = &http_msg_dispatcher_;
        logic_ctx.scheduler = &scheduler_;
        logic_ctx.local_logic = local_logic_;
        logic_ctx.thread_ev_base = thread_->GetThreadEvBase();

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_INFO("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}

void ThreadSink::OnTransReturn(ThreadInterface* source_thread, const ConnGuid* conn_guid, const MsgHead& msg_head,
                               const void* msg_body, size_t msg_body_len)
{
    TransId trans_id = msg_head.passback;
    TransCtx* trans_ctx = thread_->GetTransCenter()->GetTransCtx(trans_id);
    if (NULL == trans_ctx)
    {
        LOG_WARN("failed to get trans ctx by trans id: " << trans_id << ", maybe canceled");
        return;
    }

    if (trans_ctx->sink != NULL)
    {
        MsgHead passback_msg_head = msg_head;
        passback_msg_head.passback = trans_ctx->passback;

        Peer peer(PEER_TYPE_THREAD, source_thread->GetThreadName(), source_thread->GetThreadIdx());
        trans_ctx->sink->OnRecvRsp(trans_id, peer, passback_msg_head, msg_body, msg_body_len,
                                   trans_ctx->async_data, trans_ctx->async_data_len);
    }

    thread_->GetTransCenter()->CancelTrans(trans_id);
}
}
