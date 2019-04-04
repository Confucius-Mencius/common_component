#include "global_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "version.h"

namespace global
{
Threads::Threads() : threads_ctx_(), related_thread_groups_()
{
    global_thread_group_ = nullptr;
    global_thread_sink_ = nullptr;
}

Threads::~Threads()
{
}

const char* Threads::GetVersion() const
{
    return GLOBAL_THREAD_GLOBAL_THREAD_VERSION;
}

const char* Threads::GetLastErrMsg() const
{
    return nullptr;
}

void Threads::Release()
{
    SAFE_RELEASE(global_thread_group_);
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
    SAFE_FINALIZE(global_thread_group_);
}

int Threads::Activate()
{
    if (SAFE_ACTIVATE_FAILED(global_thread_group_))
    {
        return -1;
    }

    return 0;
}

void Threads::Freeze()
{
    SAFE_FREEZE(global_thread_group_);
}

int Threads::CreateThreadGroup()
{
    int ret = -1;

    do
    {
        ThreadGroupCtx thread_group_ctx;
        thread_group_ctx.common_component_dir = threads_ctx_.common_component_dir;
        thread_group_ctx.enable_cpu_profiling = threads_ctx_.app_frame_conf_mgr->EnableCPUProfiling();
        thread_group_ctx.thread_name = "global thread";
        thread_group_ctx.thread_count = 1;
        thread_group_ctx.thread_sink_creator = ThreadSink::Create;
        thread_group_ctx.threads_ctx = &threads_ctx_;

        global_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup(&thread_group_ctx);
        if (nullptr == global_thread_group_)
        {
            break;
        }

        global_thread_sink_ = static_cast<ThreadSink*>(global_thread_group_->GetThread(0)->GetThreadSink());

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (global_thread_group_ != nullptr)
        {
            SAFE_DESTROY(global_thread_group_);
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

    if (global_thread_sink_ != nullptr)
    {
        global_thread_sink_->SetRelatedThreadGroups(&related_thread_groups_);
    }
}

ThreadGroupInterface* Threads::GetGlobalThreadGroup() const
{
    return global_thread_group_;
}

LogicInterface* Threads::GetLogic() const
{
    return global_thread_sink_->GetLogic();
}

void Threads::SetReloadFinish(bool finished)
{
    global_thread_sink_->SetReloadFinish(finished);
}

bool Threads::ReloadFinished()
{
    return global_thread_sink_->ReloadFinished();
}
}
