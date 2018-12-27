#include "tcp_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "container_util.h"
#include "str_util.h"
#include "version.h"

namespace tcp
{
Threads::Threads() : threads_ctx_(), tcp_thread_vec_(), tcp_thread_sink_vec_(), related_thread_group_()
{
    listen_thread_group_ = NULL;
    tcp_thread_group_ = NULL;
    listen_thread_ = NULL;
    listen_thread_sink_ = NULL;
}

Threads::~Threads()
{
}

const char* Threads::GetVersion() const
{
    return TCP_THREADS_VERSION;
}

const char* Threads::GetLastErrMsg() const
{
    return NULL;
}

void Threads::Release()
{
    RELEASE_CONTAINER(tcp_thread_sink_vec_);
    SAFE_RELEASE(listen_thread_sink_);
    delete this;
}

int Threads::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    threads_ctx_ = *((ThreadsCtx*) ctx);
    return 0;
}

void Threads::Finalize()
{
    // 由thread center集中管理
}

int Threads::Activate()
{
    if (listen_thread_group_->Activate() != 0)
    {
        return -1;
    }

    if (tcp_thread_group_->Activate() != 0)
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
        listen_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup();
        if (NULL == listen_thread_group_)
        {
            break;
        }

        tcp_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup();
        if (NULL == tcp_thread_group_)
        {
            break;
        }

        if (CreateListenThread() != 0)
        {
            break;
        }

        if (CreateTcpThreads() != 0)
        {
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (tcp_thread_group_ != NULL)
        {
            SAFE_DESTROY(tcp_thread_group_);
        }

        if (listen_thread_group_ != NULL)
        {
            SAFE_DESTROY(listen_thread_group_);
        }
    }

    return ret;
}

ThreadGroupInterface* Threads::GetListenThreadGroup() const
{
    return listen_thread_group_;
}

ThreadGroupInterface* Threads::GetTCPThreadGroup() const
{
    return tcp_thread_group_;
}

void Threads::SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_group)
{
    if (NULL == related_thread_group)
    {
        return;
    }

    related_thread_group_ = *related_thread_group;

    for (TcpThreadSinkVec::iterator it = tcp_thread_sink_vec_.begin(); it != tcp_thread_sink_vec_.end(); ++it)
    {
        (*it)->SetRelatedThreadGroup(&related_thread_group_);
    }
}

int Threads::CreateListenThread()
{
    ThreadCtx thread_ctx;
    thread_ctx.common_component_dir = threads_ctx_.common_component_dir;

    thread_ctx.name = "tcp listen thread";
    thread_ctx.idx = 0;

    ListenThreadSink* sink = ListenThreadSink::Create();
    if (NULL == sink)
    {
        const int err = errno;
        LOG_ERROR("failed to create listen thread sink, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    sink->SetThreadsCtx(&threads_ctx_);
    sink->SetTCPThreadGroup(tcp_thread_group_);
    thread_ctx.sink = sink;

    ThreadInterface* thread = listen_thread_group_->CreateThread(&thread_ctx);
    if (NULL == thread)
    {
        return -1;
    }

    listen_thread_ = thread;
    listen_thread_sink_ = sink;

    return 0;
}

int Threads::CreateTcpThreads()
{
    ThreadCtx thread_ctx;
    ThreadSink* sink = NULL;
    char thread_name[64] = "";
    const int tcp_thread_count = threads_ctx_.raw ? threads_ctx_.conf_mgr->GetRawTcpThreadCount()
                                 : threads_ctx_.conf_mgr->GetTcpThreadCount();

    for (int i = 0; i < tcp_thread_count; ++i)
    {
        thread_ctx.common_component_dir = threads_ctx_.common_component_dir;
        thread_ctx.need_reply_msg_check_interval = threads_ctx_.conf_mgr->GetPeerNeedReplyMsgCheckInterval();

        if (!threads_ctx_.raw)
        {
            StrPrintf(thread_name, sizeof(thread_name), "tcp thread #%d", i);
        }
        else
        {
            StrPrintf(thread_name, sizeof(thread_name), "raw tcp thread #%d", i);
        }

        thread_ctx.name = thread_name;
        thread_ctx.idx = i;

        sink = ThreadSink::Create();
        if (NULL == sink)
        {
            const int err = errno;
            LOG_ERROR("failed to create tcp thread sink, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }

        sink->SetThreadsCtx(&threads_ctx_);
        sink->SetListenThread(listen_thread_group_->GetThread(0));
        sink->SetTcpThreadGroup(tcp_thread_group_);
        thread_ctx.sink = sink;

        ThreadInterface* thread = tcp_thread_group_->CreateThread(&thread_ctx);
        if (NULL == thread)
        {
            return -1;
        }

        tcp_thread_vec_.push_back(thread);
        tcp_thread_sink_vec_.push_back(sink);
    }

    return 0;
}
}
