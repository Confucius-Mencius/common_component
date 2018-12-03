#include "http_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "container_util.h"
#include "http_threads_sync_facility.h"
#include "str_util.h"
#include "version.h"

namespace http
{
Threads::Threads() : threads_ctx_(), http_thread_vec_(), http_thread_sink_vec_(), related_thread_group_()
{
    http_thread_group_ = NULL;
    http_listen_sock_fd_ = -1;

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    https_listen_sock_fd_ = -1;
#endif
}

Threads::~Threads()
{
}

const char* Threads::GetVersion() const
{
    return HTTP_THREADS_HTTP_THREADS_VERSION;
}

const char* Threads::GetLastErrMsg() const
{
    return NULL;
}

void Threads::Release()
{
    RELEASE_CONTAINER(http_thread_sink_vec_);
    delete this;
}

int Threads::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    threads_ctx_ = *((ThreadsCtx*) ctx);

    pthread_mutex_init(&g_online_http_conn_count_mutex, NULL);

    std::string addr;
    unsigned short port;

    if (threads_ctx_.conf_mgr->GetHttpAddrPort().length() > 0)
    {
        if (ParseAddPort(addr, port, threads_ctx_.conf_mgr->GetHttpAddrPort()) != 0)
        {
            LOG_ERROR("failed to parse addr port: " << threads_ctx_.conf_mgr->GetHttpAddrPort());
            return -1;
        }

        LOG_INFO("http listen addr: " << addr << ", port: " << port);

        http_listen_sock_fd_ = evhttp_create_listen_socket(addr.c_str(), port, 0xffff); // TODO backlog
        if (http_listen_sock_fd_ < 0)
        {
            const int err = errno;
            LOG_ERROR("failed to create http listen socket, errno: " << err << ", err msg: " << strerror(err)
                      << ", http listen socket fd: " << http_listen_sock_fd_);
            return -1;
        }
    }

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    if (threads_ctx_.conf_mgr->GetHttpsAddrPort().length() > 0)
    {
        if (ParseAddPort(addr, port, threads_ctx_.conf_mgr->GetHttpsAddrPort()) != 0)
        {
            LOG_ERROR("failed to parse addr port: " << threads_ctx_.conf_mgr->GetHttpsAddrPort());
            return -1;
        }

        LOG_INFO("https listen addr: " << addr << ", port: " << port);

        https_listen_sock_fd_ = evhttp_create_listen_socket(addr.c_str(), port, 0xffff);
        if (https_listen_sock_fd_ < 0)
        {
            const int err = errno;
            LOG_ERROR("failed to create https listen socket, errno: " << err << ", err msg: " << strerror(err)
                      << ", https listen socket fd: " << https_listen_sock_fd_);
            return -1;
        }
    }
#endif

    return 0;
}

void Threads::Finalize()
{
    // 由thread center集中管理

#if LIBEVENT_VERSION_NUMBER >= 0x2010500
    if (https_listen_sock_fd_ >= 0)
    {
        evutil_closesocket(https_listen_sock_fd_);
        https_listen_sock_fd_ = -1;
    }
#endif

    if (http_listen_sock_fd_ >= 0)
    {
        evutil_closesocket(http_listen_sock_fd_);
        http_listen_sock_fd_ = -1;
    }

    pthread_mutex_destroy(&g_online_http_conn_count_mutex);
}

int Threads::Activate()
{
    if (http_thread_group_->Activate() != 0)
    {
        return -1;
    }

    return 0;
}

void Threads::Freeze()
{
    // 由thread center集中管理
}

int Threads::CreateThreadGroup()
{
    int ret = -1;

    do
    {
        http_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup();
        if (NULL == http_thread_group_)
        {
            break;
        }

        if (CreateHttpThreads() != 0)
        {
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (http_thread_group_ != NULL)
        {
            SAFE_DESTROY(http_thread_group_);
        }
    }

    return ret;
}

ThreadGroupInterface* Threads::GetHttpThreadGroup() const
{
    return http_thread_group_;
}

void Threads::SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group)
{
    if (NULL == related_thread_group)
    {
        return;
    }

    related_thread_group_ = *related_thread_group;

    for (HttpThreadSinkVec::iterator it = http_thread_sink_vec_.begin(); it != http_thread_sink_vec_.end(); ++it)
    {
        (*it)->SetRelatedThreadGroup(&related_thread_group_);
    }
}

int Threads::CreateHttpThreads()
{
    ThreadCtx thread_ctx;
    ThreadSink* sink = NULL;
    char thread_name[64] = "";

    for (int i = 0; i < threads_ctx_.conf_mgr->GetHttpThreadCount(); ++i)
    {
        thread_ctx.common_component_dir = threads_ctx_.common_component_dir;
        thread_ctx.need_reply_msg_check_interval = threads_ctx_.conf_mgr->GetPeerNeedReplyMsgCheckInterval();

        StrPrintf(thread_name, sizeof(thread_name), "http thread #%d", i);
        thread_ctx.name = thread_name;
        thread_ctx.idx = i;

        sink = ThreadSink::Create();
        if (NULL == sink)
        {
            const int err = errno;
            LOG_ERROR("failed to create http thread sink, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }

        sink->SetThreadsCtx(&threads_ctx_);
        sink->SetHttpThreadGroup(http_thread_group_);
        sink->SetListenSocketFd(http_listen_sock_fd_, https_listen_sock_fd_);
        thread_ctx.sink = sink;

        ThreadInterface* thread = http_thread_group_->CreateThread(&thread_ctx);
        if (NULL == thread)
        {
            return -1;
        }

        http_thread_vec_.push_back(thread);
        http_thread_sink_vec_.push_back(sink);
    }

    return 0;
}
}
