#include "thread_center.h"
#include "container_util.h"

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
    return NULL;
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

ThreadGroupInterface* ThreadCenter::CreateThreadGroup()
{
    // 注意，create的时候要把创建出的对象激活后再交给使用者，不要让使用者再去调用Initialize和Activate。对于需要的参数，可以通过增加参数的方式传进来。
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
        if (thread_group->Initialize(NULL) != 0)
        {
            break;
        }

        // not activated here

        if (!thread_group_set_.insert(thread_group).second)
        {
            const int err = errno;
            LOG_ERROR("failed to insert to set, thread group: " << thread_group
                          << ", errno: " << err << ", err msg: " << strerror(err));
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        thread_group_set_.erase(thread_group);
        SAFE_DESTROY(thread_group);
        return NULL;
    }

    return thread_group;
}
} // namespace thread_center
