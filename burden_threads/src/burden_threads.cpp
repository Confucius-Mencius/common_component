#include "burden_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "container_util.h"
#include "str_util.h"
#include "version.h"

namespace burden
{
Threads::Threads() : threads_ctx_(), burden_thread_vec_(), burden_thread_sink_vec_(), related_thread_group_()
{
    burden_thread_group_ = NULL;
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
    return NULL;
}

void Threads::Release()
{
    RELEASE_CONTAINER(burden_thread_sink_vec_);
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
    if (burden_thread_group_->Activate() != 0)
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
        if (CreateBurdenThreads() != 0)
        {
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (burden_thread_group_ != NULL)
        {
            SAFE_DESTROY(burden_thread_group_);
        }
    }

    return ret;
}

ThreadGroupInterface* Threads::GetBurdenThreadGroup() const
{
    return burden_thread_group_;
}

void Threads::SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group)
{
    if (NULL == related_thread_group)
    {
        return;
    }

    related_thread_group_ = *related_thread_group;

    for (BurdenThreadSinkVec::iterator it = burden_thread_sink_vec_.begin(); it != burden_thread_sink_vec_.end(); ++it)
    {
        (*it)->SetRelatedThreadGroup(&related_thread_group_);
    }
}

int Threads::CreateBurdenThreads()
{
    burden_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup();
    if (NULL == burden_thread_group_)
    {
        return -1;
    }

    ThreadCtx thread_ctx;
    ThreadSink* sink = NULL;
    char thread_name[64] = "";

    for (int i = 0; i < threads_ctx_.conf_mgr->GetBurdenThreadCount(); ++i)
    {
        thread_ctx.common_component_dir = threads_ctx_.common_component_dir;
        thread_ctx.need_reply_msg_check_interval = threads_ctx_.conf_mgr->GetPeerNeedReplyMsgCheckInterval();

        StrPrintf(thread_name, sizeof(thread_name), "burden thread #%d", i);
        thread_ctx.name = thread_name;
        thread_ctx.idx = i;

        sink = ThreadSink::Create();
        if (NULL == sink)
        {
            const int err = errno;
            LOG_ERROR("failed to create burden thread sink, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }

        sink->SetThreadsCtx(&threads_ctx_);
        sink->SetBurdenThreadGroup(burden_thread_group_);
        thread_ctx.sink = sink;

        ThreadInterface* thread = burden_thread_group_->CreateThread(&thread_ctx);
        if (NULL == thread)
        {
            return -1;
        }

        burden_thread_vec_.push_back(thread);
        burden_thread_sink_vec_.push_back(sink);
    }

    return 0;
}
}
