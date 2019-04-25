#include "work_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "container_util.h"
#include "str_util.h"
#include "version.h"

namespace work
{
Threads::Threads() : threads_ctx_(), related_thread_groups_()
{
    work_thread_group_ = nullptr;
}

Threads::~Threads()
{
}

const char* Threads::GetVersion() const
{
    return WORK_THREADS_WORK_THREADS_VERSION;
}

const char* Threads::GetLastErrMsg() const
{
    return nullptr;
}

void Threads::Release()
{
    SAFE_RELEASE(work_thread_group_);
    delete this;
}

int Threads::Initialize(const void* ctx)
{
    if (nullptr == ctx)
    {
        return -1;
    }

    threads_ctx_ = *(static_cast<const ThreadsCtx*>(ctx));
    return 0;
}

void Threads::Finalize()
{
    SAFE_FINALIZE(work_thread_group_);
}

int Threads::Activate()
{
    if (SAFE_ACTIVATE_FAILED(work_thread_group_))
    {
        return -1;
    }

    return 0;
}

void Threads::Freeze()
{
    SAFE_FREEZE(work_thread_group_);
}

int Threads::CreateThreadGroup(const char* thread_name)
{
    int ret = -1;

    do
    {
        ThreadGroupCtx thread_group_ctx;
        thread_group_ctx.common_component_dir = threads_ctx_.common_component_dir;
        thread_group_ctx.enable_cpu_profiling = threads_ctx_.app_frame_conf_mgr->EnableCPUProfiling();
        thread_group_ctx.thread_name.assign(thread_name);
        thread_group_ctx.thread_count = threads_ctx_.conf.thread_count;
        thread_group_ctx.thread_sink_creator = ThreadSink::Create;
        thread_group_ctx.threads_ctx = &threads_ctx_;

        work_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup(&thread_group_ctx);
        if (nullptr == work_thread_group_)
        {
            break;
        }

        for (int i = 0; i < work_thread_group_->GetThreadCount(); ++i)
        {
            ThreadSink* thread_sink = static_cast<ThreadSink*>(work_thread_group_->GetThread(i)->GetThreadSink());
            thread_sink->SetWorkThreadGroup(work_thread_group_);
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (work_thread_group_ != nullptr)
        {
            SAFE_DESTROY(work_thread_group_);
        }
    }

    return ret;
}

void Threads::SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_groups)
{
    if (nullptr == related_thread_groups)
    {
        return;
    }

    related_thread_groups_ = *related_thread_groups;

    for (int i = 0; i < work_thread_group_->GetThreadCount(); ++i)
    {
        ThreadSink* thread_sink = static_cast<ThreadSink*>(work_thread_group_->GetThread(i)->GetThreadSink());
        thread_sink->SetRelatedThreadGroups(&related_thread_groups_);
    }
}

ThreadGroupInterface* Threads::GetWorkThreadGroup() const
{
    return work_thread_group_;
}
}
