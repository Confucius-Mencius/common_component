#include "ws_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "container_util.h"
#include "str_util.h"
#include "thread_sink.h"
#include "version.h"

namespace ws
{
static void LogEmitFunction(int level, const char* msg)
{
    switch (level)
    {
        case LLL_ERR:
        {
            LOG_ERROR(msg);
        }
        break;

        case LLL_WARN:
        {
            LOG_WARN(msg);
        }
        break;

        case LLL_NOTICE:
        {
            LOG_INFO(msg);
        }
        break;

//        case LLL_INFO: // 信息太多，先注释掉
//        {
//            LOG_DEBUG(msg);
//        }
//        break;

        default:
        {
        }
        break;
    }
}

Threads::Threads() : threads_ctx_(), related_thread_groups_(), controller_()
{
    ws_thread_group_ = NULL;
}

Threads::~Threads()
{
}

const char* Threads::GetVersion() const
{
    return WS_THREADS_VERSION;
}

const char* Threads::GetLastErrMsg() const
{
    return NULL;
}

void Threads::Release()
{
    SAFE_RELEASE(ws_thread_group_);
    delete this;
}

int Threads::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    threads_ctx_ = *(static_cast<const ThreadsCtx*>(ctx));

    LOG_ALWAYS("libwebsockets version: " << lws_get_library_version());
    lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO, LogEmitFunction);

    return 0;
}

void Threads::Finalize()
{
    controller_.Finalize();
    SAFE_FINALIZE(ws_thread_group_);
}

int Threads::Activate()
{
    if (SAFE_ACTIVATE_FAILED(ws_thread_group_) != 0)
    {
        return -1;
    }

    return 0;
}

void Threads::Freeze()
{
    SAFE_FREEZE(ws_thread_group_);
}

int Threads::CreateThreadGroup()
{
    int ret = -1;

    do
    {
        ThreadGroupCtx ws_thread_group_ctx;
        ws_thread_group_ctx.common_component_dir = threads_ctx_.common_component_dir;
        ws_thread_group_ctx.enable_cpu_profiling = threads_ctx_.conf_mgr->EnableCPUProfiling();
        ws_thread_group_ctx.thread_name = "ws thread";
        ws_thread_group_ctx.thread_count = threads_ctx_.conf_mgr->GetWSThreadCount();
        ws_thread_group_ctx.thread_sink_creator = ThreadSink::Create;
        ws_thread_group_ctx.args = &threads_ctx_;

        ws_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup(&ws_thread_group_ctx);
        if (NULL == ws_thread_group_)
        {
            break;
        }

        for (int i = 0; i < ws_thread_group_->GetThreadCount(); ++i)
        {
            ThreadSink* ws_thread_sink = static_cast<ThreadSink*>(ws_thread_group_->GetThread(i)->GetThreadSink());
            ws_thread_sink->SetWSThreadGroup(ws_thread_group_);
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (ws_thread_group_ != NULL)
        {
            SAFE_DESTROY(ws_thread_group_);
        }
    }
    else
    {
        controller_.SetWSThreadGroup(ws_thread_group_);

        if (controller_.Initialize(&threads_ctx_) != 0)
        {
            return -1;
        }
    }

    return ret;
}

ThreadGroupInterface* Threads::GetWSThreadGroup() const
{
    return ws_thread_group_;
}

void Threads::SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_groups)
{
    if (NULL == related_thread_groups)
    {
        return;
    }

    related_thread_groups_ = *related_thread_groups;

    for (int i = 0; i < ws_thread_group_->GetThreadCount(); ++i)
    {
        ThreadSink* ws_thread_sink = static_cast<ThreadSink*>(ws_thread_group_->GetThread(i)->GetThreadSink());
        ws_thread_sink->SetRelatedThreadGroups(&related_thread_groups_);
    }
}
}
