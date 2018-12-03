#include "work_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "container_util.h"
#include "str_util.h"
#include "version.h"

namespace work
{
Threads::Threads() : threads_ctx_(), work_thread_vec_(), work_thread_sink_vec_(), related_thread_group_()
{
    work_thread_group_ = NULL;
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
    return NULL;
}

void Threads::Release()
{
    RELEASE_CONTAINER(work_thread_sink_vec_);
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
    if (work_thread_group_->Activate() != 0)
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
        if (CreateWorkThreads() != 0)
        {
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (work_thread_group_ != NULL)
        {
            SAFE_DESTROY(work_thread_group_);
        }
    }

    return ret;
}

ThreadGroupInterface* Threads::GetWorkThreadGroup() const
{
    return work_thread_group_;
}

void Threads::SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group)
{
    if (NULL == related_thread_group)
    {
        return;
    }

    related_thread_group_ = *related_thread_group;

    for (WorkThreadSinkVec::iterator it = work_thread_sink_vec_.begin(); it != work_thread_sink_vec_.end(); ++it)
    {
        (*it)->SetRelatedThreadGroup(&related_thread_group_);
    }
}

int Threads::CreateWorkThreads()
{
    work_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup();
    if (NULL == work_thread_group_)
    {
        return -1;
    }

    ThreadCtx thread_ctx;
    ThreadSink* sink = NULL;
    char thread_name[64] = "";

    for (int i = 0; i < threads_ctx_.conf_mgr->GetWorkThreadCount(); ++i)
    {
        thread_ctx.common_component_dir = threads_ctx_.common_component_dir;
        thread_ctx.need_reply_msg_check_interval = threads_ctx_.conf_mgr->GetPeerNeedReplyMsgCheckInterval();

        StrPrintf(thread_name, sizeof(thread_name), "work thread #%d", i);
        thread_ctx.name = thread_name;
        thread_ctx.idx = i;

        sink = ThreadSink::Create();
        if (NULL == sink)
        {
            const int err = errno;
            LOG_ERROR("failed to create work thread sink, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }

        sink->SetThreadsCtx(&threads_ctx_);
        sink->SetWorkThreadGroup(work_thread_group_);
        thread_ctx.sink = sink;

        ThreadInterface* thread = work_thread_group_->CreateThread(&thread_ctx);
        if (NULL == thread)
        {
            return -1;
        }

        work_thread_vec_.push_back(thread);
        work_thread_sink_vec_.push_back(sink);
    }

    return 0;
}
}
