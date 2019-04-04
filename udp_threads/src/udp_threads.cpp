#include "udp_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "container_util.h"
#include "str_util.h"
#include "version.h"

namespace udp
{
Threads::Threads() : threads_ctx_(), udp_thread_vec_(), udp_thread_sink_vec_(), related_thread_group_()
{
    udp_thread_group_ = nullptr;
}

Threads::~Threads()
{
}

const char* Threads::GetVersion() const
{
    return UDP_THREADS_UDP_THREADS_VERSION;
}

const char* Threads::GetLastErrMsg() const
{
    return nullptr;
}

void Threads::Release()
{
    RELEASE_CONTAINER(udp_thread_sink_vec_);
    delete this;
}

int Threads::Initialize(const void* ctx)
{
    if (nullptr == ctx)
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
    if (udp_thread_group_->Activate() != 0)
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
        udp_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup();
        if (nullptr == udp_thread_group_)
        {
            break;
        }

        if (CreateUdpThreads() != 0)
        {
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (udp_thread_group_ != nullptr)
        {
            SAFE_DESTROY(udp_thread_group_);
        }
    }

    return ret;
}

ThreadGroupInterface* Threads::GetUdpThreadGroup() const
{
    return udp_thread_group_;
}

void Threads::SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group)
{
    if (nullptr == related_thread_group)
    {
        return;
    }

    related_thread_group_ = *related_thread_group;

    for (UdpThreadSinkVec::iterator it = udp_thread_sink_vec_.begin(); it != udp_thread_sink_vec_.end(); ++it)
    {
        (*it)->SetRelatedThreadGroup(&related_thread_group_);
    }
}

int Threads::CreateUdpThreads()
{
    ThreadCtx thread_ctx;
    ThreadSink* sink = nullptr;
    char thread_name[64] = "";

    for (int i = 0; i < threads_ctx_.conf_mgr->GetUdpThreadCount(); ++i)
    {
        thread_ctx.common_component_dir = threads_ctx_.common_component_dir;
        thread_ctx.need_reply_msg_check_interval = threads_ctx_.conf_mgr->GetPeerNeedReplyMsgCheckInterval();

        StrPrintf(thread_name, sizeof(thread_name), "udp thread #%d", i);
        thread_ctx.name = thread_name;
        thread_ctx.idx = i;

        sink = ThreadSink::Create();
        if (nullptr == sink)
        {
            const int err = errno;
            LOG_ERROR("failed to create udp thread sink, errno: " << err << ", err msg: " << strerror(err));
            return -1;
        }

        sink->SetThreadsCtx(&threads_ctx_);
        sink->SetUdpThreadGroup(udp_thread_group_);
        thread_ctx.sink = sink;

        ThreadInterface* thread = udp_thread_group_->CreateThread(&thread_ctx);
        if (nullptr == thread)
        {
            return -1;
        }

        udp_thread_vec_.push_back(thread);
        udp_thread_sink_vec_.push_back(sink);
    }

    return 0;
}
}
