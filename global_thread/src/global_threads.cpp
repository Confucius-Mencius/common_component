#include "global_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "version.h"

namespace global
{
Threads::Threads() : threads_ctx_(), related_thread_group_()
{
    global_thread_group_ = NULL;
    global_thread_ = NULL;
    global_thread_sink_ = NULL;
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
    return NULL;
}

void Threads::Release()
{
    SAFE_RELEASE(global_thread_sink_);
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
    if (global_thread_group_->Activate() != 0)
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
        if (CreateGlobalThread() != 0)
        {
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (global_thread_group_ != NULL)
        {
            SAFE_DESTROY(global_thread_group_);
        }
    }

    return ret;
}

ThreadGroupInterface* Threads::GetGlobalThreadGroup() const
{
    return global_thread_group_;
}

LogicInterface* Threads::GetLogic() const
{
    return global_thread_sink_->GetLogic();
}

void Threads::SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group)
{
    if (NULL == related_thread_group)
    {
        return;
    }

    related_thread_group_ = *related_thread_group;

    if (global_thread_sink_ != NULL)
    {
        global_thread_sink_->SetRelatedThreadGroup(&related_thread_group_);
    }
}

int Threads::CreateGlobalThread()
{
    global_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup();
    if (NULL == global_thread_group_)
    {
        return -1;
    }

    ThreadCtx thread_ctx;
    ThreadSink* sink = NULL;

    thread_ctx.common_component_dir = threads_ctx_.common_component_dir;
    thread_ctx.need_reply_msg_check_interval = threads_ctx_.conf_mgr->GetPeerNeedReplyMsgCheckInterval();

    thread_ctx.name = "global thread";
    thread_ctx.idx = 0;

    sink = ThreadSink::Create();
    if (NULL == sink)
    {
        const int err = errno;
        LOG_ERROR("failed to create global thread sink, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    sink->SetThreadsCtx(&threads_ctx_);
    thread_ctx.sink = sink;

    global_thread_ = global_thread_group_->CreateThread(&thread_ctx);
    if (NULL == global_thread_)
    {
        return -1;
    }

    global_thread_sink_ = sink;
    return 0;
}

void Threads::SetReloadFinish(bool finish)
{
    global_thread_sink_->SetReloadFinish(finish);
}

bool Threads::ReloadFinished()
{
    return global_thread_sink_->ReloadFinished();
}
}
