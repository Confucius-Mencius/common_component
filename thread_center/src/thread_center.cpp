#include "thread_center.h"
#include "container_util.h"
#include "str_util.h"
#include "version.h"

namespace thread_center
{
ThreadCenter::ThreadCenter() : thread_group_set_()
{
}

ThreadCenter::~ThreadCenter()
{
}

const char* ThreadCenter::GetVersion() const
{
    return THREAD_CENTER_VERSION;
}

const char* ThreadCenter::GetLastErrMsg() const
{
    return NULL;
}

void ThreadCenter::Release()
{
    RELEASE_CONTAINER(thread_group_set_);
    delete this;
}

int ThreadCenter::Initialize(const void* ctx)
{
    (void) ctx;
    return 0;
}

void ThreadCenter::Finalize()
{
    FINALIZE_CONTAINER(thread_group_set_);
}

int ThreadCenter::Activate()
{
    return 0;
}

void ThreadCenter::Freeze()
{
    FREEZE_CONTAINER(thread_group_set_);
}

ThreadGroupInterface* ThreadCenter::CreateThreadGroup(const ThreadGroupCtx* ctx)
{
    ThreadGroup* thread_group = ThreadGroup::Create();
    if (NULL == thread_group)
    {
        const int err = errno;
        LOG_ERROR("failed to create thread group, errno: " << err << ", err msg: " << strerror(err));
        return NULL;
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

            char thread_name[64] = "";
            StrPrintf(thread_name, sizeof(thread_name), "%s #%d", ctx->thread_name.c_str(), i);
            thread_ctx.name.assign(thread_name);

            thread_ctx.idx = i;
            thread_ctx.sink = ctx->thread_sink_creator();
            thread_ctx.args = ctx->args;

            if (NULL == thread_group->CreateThread(&thread_ctx))
            {
                LOG_ERROR("failed to create thread, i: " << i);
                SAFE_DESTROY(thread_group);
                return NULL;
            }
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        SAFE_DESTROY(thread_group);
        return NULL;
    }

    thread_group_set_.insert(thread_group);
    return thread_group;
}
} // namespace thread_center
