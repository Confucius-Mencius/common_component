#include "thread_center.h"
#include "str_util.h"
#include "version.h"

pthread_key_t g_tsd_key;

namespace thread_center
{
ThreadCenter::ThreadCenter()
{
}

ThreadCenter::~ThreadCenter()
{
}

const char* ThreadCenter::GetVersion() const
{
    return THREAD_CENTER_THREAD_CENTER_VERSION;
}

const char* ThreadCenter::GetLastErrMsg() const
{
    return nullptr;
}

void ThreadCenter::Release()
{
    delete this;
}

int ThreadCenter::Initialize(const void* ctx)
{
    return 0;
}

void ThreadCenter::Finalize()
{
}

int ThreadCenter::Activate()
{
    return 0;
}

void ThreadCenter::Freeze()
{
}

ThreadGroupInterface* ThreadCenter::CreateThreadGroup(const ThreadGroupCtx* ctx)
{
    ThreadGroup* thread_group = ThreadGroup::Create();
    if (nullptr == thread_group)
    {
        const int err = errno;
        LOG_ERROR("failed to create thread group, errno: " << err << ", err msg: " << strerror(err));
        return nullptr;
    }

    thread_group->SetThreadCenter(this);
    int ret = -1;

    do
    {
        if (thread_group->Initialize(ctx) != 0)
        {
            LOG_ERROR("thread group initialize failed");
            break;
        }

        for (int i = 0; i < ctx->thread_count; ++i)
        {
            ThreadCtx thread_ctx;
            thread_ctx.common_component_dir = ctx->common_component_dir;
            thread_ctx.enable_cpu_profiling = ctx->enable_cpu_profiling;
            char thread_name[64] = "";
            StrPrintf(thread_name, sizeof(thread_name), "%s #%d", ctx->thread_name.c_str(), i);
            thread_ctx.name.assign(thread_name);

            thread_ctx.idx = i;
            thread_ctx.sink = ctx->thread_sink_creator();
            thread_ctx.threads_ctx = ctx->threads_ctx;
            thread_ctx.thread_group = thread_group;

            if (nullptr == thread_group->CreateThread(&thread_ctx))
            {
                LOG_ERROR("failed to create thread, i: " << i);
                SAFE_DESTROY(thread_group);
                return nullptr;
            }
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        SAFE_DESTROY(thread_group);
        return nullptr;
    }

    return thread_group;
}
} // namespace thread_center
