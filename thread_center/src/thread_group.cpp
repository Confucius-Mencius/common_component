#include "thread_group.h"
#include "container_util.h"
#include "num_util.h"
#include "thread_center.h"

namespace thread_center
{
ThreadGroup::ThreadGroup() : thread_vec_()
{
    thread_center_ = nullptr;
}

ThreadGroup::~ThreadGroup()
{
}

const char* ThreadGroup::GetVersion() const
{
    return nullptr;
}

const char* ThreadGroup::GetLastErrMsg() const
{
    return nullptr;
}

void ThreadGroup::Release()
{
    RELEASE_CONTAINER(thread_vec_);
    delete this;
}

int ThreadGroup::Initialize(const void* ctx)
{
    if (pthread_key_create(&tsd_key_, nullptr) != 0)
    {
        const int err = errno;
        LOG_ERROR("pthread_key_create failed, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    return 0;
}

void ThreadGroup::Finalize()
{
    FINALIZE_CONTAINER(thread_vec_);
    pthread_key_delete(tsd_key_);
}

int ThreadGroup::Activate()
{
    return ACTIVATE_CONTAINER(thread_vec_);
}

void ThreadGroup::Freeze()
{
    FREEZE_CONTAINER(thread_vec_);
}

ThreadInterface* ThreadGroup::GetThread(int thread_idx) const
{
    if (INVALID_IDX(thread_idx, 0, thread_vec_.size()))
    {
        LOG_ERROR("invalid thread idx: " << thread_idx);
        return nullptr;
    }

    return thread_vec_[thread_idx];
}

pthread_key_t& ThreadGroup::GetSpecificDataKey()
{
    return tsd_key_;
}

int ThreadGroup::Start()
{
    for (ThreadVec::iterator it = thread_vec_.begin(); it != thread_vec_.end(); ++it)
    {
        if ((*it)->Start() != 0)
        {
            return -1;
        }
    }

    return 0;
}

void ThreadGroup::Join()
{
    for (ThreadVec::iterator it = thread_vec_.begin(); it != thread_vec_.end(); ++it)
    {
        (*it)->Join();
    }
}

void ThreadGroup::NotifyStop()
{
    for (ThreadVec::iterator it = thread_vec_.begin(); it != thread_vec_.end(); ++it)
    {
        (*it)->NotifyStop();
    }
}

void ThreadGroup::NotifyReload()
{
    for (ThreadVec::iterator it = thread_vec_.begin(); it != thread_vec_.end(); ++it)
    {
        (*it)->NotifyReload();
    }
}

bool ThreadGroup::CanExit() const
{
    int can_exit = 1;

    for (ThreadVec::const_iterator it = thread_vec_.cbegin(); it != thread_vec_.cend(); ++it)
    {
        can_exit &= ((*it)->CanExit() ? 1 : 0);
    }

    return (can_exit != 0);
}

void ThreadGroup::NotifyExit()
{
    for (ThreadVec::iterator it = thread_vec_.begin(); it != thread_vec_.end(); ++it)
    {
        (*it)->NotifyExit();
    }
}

int ThreadGroup::PushTaskToThread(ThreadTask* task, int thread_idx)
{
    if (nullptr == task)
    {
        return -1;
    }

    if (-1 == thread_idx)
    {
        // 广播给线程组中的所有线程
        for (ThreadVec::iterator it = thread_vec_.begin(); it != thread_vec_.end(); ++it)
        {
            (*it)->PushTask(task->Clone());
        }

        task->Release();
        return 0;
    }

    if (INVALID_IDX(thread_idx, 0, thread_vec_.size()))
    {
        LOG_ERROR("invalid thread idx: " << thread_idx);
        return -1;
    }

    thread_vec_[thread_idx]->PushTask(task);
    return 0;
}

ThreadInterface* ThreadGroup::CreateThread(const ThreadCtx* thread_ctx)
{
    Thread* thread = Thread::Create();
    if (nullptr == thread)
    {
        const int err = errno;
        LOG_ERROR("failed to create thread, errno: " << err << ", err msg: " << strerror(err));
        return nullptr;
    }

    int ret = -1;

    do
    {
        if (thread->Initialize(thread_ctx) != 0)
        {
            LOG_ERROR(thread->GetThreadName() << " initialize failed");
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        SAFE_DESTROY(thread);
        return nullptr;
    }

    thread_vec_.push_back(thread);
    return thread;
}
} // namespace thread_center
