#include "global_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "mem_util.h"
#include "str_util.h"
#include "version.h"

namespace global
{
Threads::Threads() : threads_ctx_(), related_thread_groups_(), work_threads_loader_(), global_logic_args_()
{
    work_threads_ = nullptr;
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
    if (work_threads_->CreateThreadGroup("global") != 0)
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

ThreadGroupInterface* Threads::GetGlobalThreadGroup() const
{
    return work_threads_->GetWorkThreadGroup();
}

LogicInterface* Threads::GetLogic() const
{
    return global_logic_args_.global_logic;
}

int Threads::LoadWorkThreads()
{
    char WORK_THREADS_SO_PATH[MAX_PATH_LEN] = "";
    StrPrintf(WORK_THREADS_SO_PATH, sizeof(WORK_THREADS_SO_PATH), "%s/libwork_threads.so",
              threads_ctx_.common_component_dir);

    if (work_threads_loader_.Load(WORK_THREADS_SO_PATH) != 0)
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
    work_threads_ctx.conf.thread_count = 1;
    work_threads_ctx.conf.common_logic_so = std::string(threads_ctx_.common_component_dir) + "/libglobal_work_common_logic.so";

    global_logic_args_.app_frame_conf_mgr = threads_ctx_.app_frame_conf_mgr;
    global_logic_args_.related_thread_groups = &related_thread_groups_;

    work_threads_ctx.logic_args = &global_logic_args_;

    if (work_threads_->Initialize(&work_threads_ctx) != 0)
    {
        return -1;
    }

    return 0;
}
}
