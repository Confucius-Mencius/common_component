#include "burden_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "mem_util.h"
#include "str_util.h"
#include "version.h"

namespace burden
{
Threads::Threads() : threads_ctx_(), related_thread_groups_(), work_threads_loader_(), burden_logic_args_()
{
    work_threads_ = nullptr;
}

Threads::~Threads()
{
}

const char* Threads::GetVersion() const
{
    return BURDEN_THREADS_BURDEN_THREADS_VERSION;
}

const char* Threads::GetLastErrMsg() const
{
    return nullptr;
}

void Threads::Release()
{
    SAFE_RELEASE(work_threads_);
    delete this;
}

int Threads::Initialize(const void* ctx)
{
    if (nullptr == ctx)
    {
        return -1;
    }

    threads_ctx_ = *(static_cast<const work::ThreadsCtx*>(ctx));

    if (LoadWorkThreads() != 0)
    {
        return -1;
    }

    return 0;
}

void Threads::Finalize()
{
    SAFE_FINALIZE(work_threads_);
}

int Threads::Activate()
{
    if (SAFE_ACTIVATE_FAILED(work_threads_))
    {
        return -1;
    }

    return 0;
}

void Threads::Freeze()
{
    SAFE_FREEZE(work_threads_);
}

int Threads::CreateThreadGroup()
{
    if (work_threads_->CreateThreadGroup("burden thread") != 0)
    {
        return -1;
    }

    return 0;
}

void Threads::SetRelatedThreadGroups(const work::RelatedThreadGroups* related_thread_groups)
{
    if (nullptr == related_thread_groups)
    {
        return;
    }

    work_threads_->SetRelatedThreadGroups(related_thread_groups);
    related_thread_groups_ = *related_thread_groups;
}

ThreadGroupInterface* Threads::GetBurdenThreadGroup() const
{
    return work_threads_->GetWorkThreadGroup();
}

int Threads::LoadWorkThreads()
{
    char work_threads_so_path[MAX_PATH_LEN] = "";
    StrPrintf(work_threads_so_path, sizeof(work_threads_so_path), "%s/libwork_threads.so",
              threads_ctx_.common_component_dir);

    if (work_threads_loader_.Load(work_threads_so_path) != 0)
    {
        LOG_ERROR(work_threads_loader_.GetLastErrMsg());
        return -1;
    }

    work_threads_ = static_cast<work::ThreadsInterface*>(work_threads_loader_.GetModuleInterface());
    if (nullptr == work_threads_)
    {
        LOG_ERROR(work_threads_loader_.GetLastErrMsg());
        return -1;
    }

    work::ThreadsCtx work_threads_ctx = threads_ctx_;
    work_threads_ctx.conf.thread_count = threads_ctx_.app_frame_conf_mgr->GetBurdenThreadCount();
    work_threads_ctx.conf.logic_so_group.push_back(std::string(threads_ctx_.common_component_dir) + "/libburden_work_logic.so");

    burden_logic_args_.app_frame_conf_mgr = threads_ctx_.app_frame_conf_mgr;
    burden_logic_args_.related_thread_groups = &related_thread_groups_;

    work_threads_ctx.logic_args = &burden_logic_args_;

    if (work_threads_->Initialize(&work_threads_ctx) != 0)
    {
        return -1;
    }

    return 0;
}
}
