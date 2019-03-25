#include "tcp_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "listen_thread_sink.h"
#include "str_util.h"
#include "thread_sink.h"
#include "version.h"

namespace tcp
{
namespace raw
{
Threads::Threads() : threads_ctx_(), related_thread_groups_()
{
    listen_thread_group_ = NULL;
    tcp_thread_group_ = NULL;
}

Threads::~Threads()
{
}

const char* Threads::GetVersion() const
{
    return RAW_TCP_THREADS_RAW_TCP_THREADS_VERSION;
}

const char* Threads::GetLastErrMsg() const
{
    return NULL;
}

void Threads::Release()
{
    SAFE_RELEASE(tcp_thread_group_);
    SAFE_RELEASE(listen_thread_group_);
    delete this;
}

int Threads::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    threads_ctx_ = *(static_cast<const ThreadsCtx*>(ctx));
    return 0;
}

void Threads::Finalize()
{
    SAFE_FINALIZE(tcp_thread_group_);
    SAFE_FINALIZE(listen_thread_group_);
}

int Threads::Activate()
{
    if (SAFE_ACTIVATE_FAILED(listen_thread_group_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(tcp_thread_group_) != 0)
    {
        return -1;
    }

    return 0;
}

void Threads::Freeze()
{
    SAFE_FREEZE(tcp_thread_group_);
    SAFE_FREEZE(listen_thread_group_);
}

int Threads::CreateThreadGroup(const char* name_prefix)
{
    int ret = -1;

    do
    {
        ThreadGroupCtx listen_thread_group_ctx;
        listen_thread_group_ctx.common_component_dir = threads_ctx_.common_component_dir;
        listen_thread_group_ctx.enable_cpu_profiling = threads_ctx_.app_frame_conf_mgr->EnableCPUProfiling();
        listen_thread_group_ctx.thread_name = std::string(name_prefix) + " listen thread";
        listen_thread_group_ctx.thread_count = 1;
        listen_thread_group_ctx.thread_sink_creator = ListenThreadSink::Create;
        listen_thread_group_ctx.args = &threads_ctx_;

        listen_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup(&listen_thread_group_ctx);
        if (NULL == listen_thread_group_)
        {
            break;
        }

        ThreadGroupCtx tcp_thread_group_ctx;
        tcp_thread_group_ctx.common_component_dir = threads_ctx_.common_component_dir;
        tcp_thread_group_ctx.enable_cpu_profiling = threads_ctx_.app_frame_conf_mgr->EnableCPUProfiling();
        tcp_thread_group_ctx.thread_name = std::string(name_prefix) + " thread";
        tcp_thread_group_ctx.thread_count = threads_ctx_.conf.thread_count;
        tcp_thread_group_ctx.thread_sink_creator = ThreadSink::Create;
        tcp_thread_group_ctx.args = &threads_ctx_;

        tcp_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup(&tcp_thread_group_ctx);
        if (NULL == tcp_thread_group_)
        {
            break;
        }

        ThreadInterface* listen_thread = listen_thread_group_->GetThread(0);
        ListenThreadSink* listen_thread_sink = static_cast<ListenThreadSink*>(listen_thread->GetThreadSink());
        listen_thread_sink->SetTCPThreadGroup(tcp_thread_group_);

        for (int i = 0; i < tcp_thread_group_->GetThreadCount(); ++i)
        {
            ThreadSink* thread_sink = static_cast<ThreadSink*>(tcp_thread_group_->GetThread(i)->GetThreadSink());
            thread_sink->SetListenThread(listen_thread);
            thread_sink->SetTCPThreadGroup(tcp_thread_group_);
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

void Threads::SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_groups)
{
    if (NULL == related_thread_groups)
    {
        return;
    }

    related_thread_groups_ = *related_thread_groups;

    for (int i = 0; i < tcp_thread_group_->GetThreadCount(); ++i)
    {
        ThreadSink* tcp_thread_sink = static_cast<ThreadSink*>(tcp_thread_group_->GetThread(i)->GetThreadSink());
        tcp_thread_sink->SetRelatedThreadGroups(&related_thread_groups_);
    }
}
}
}
