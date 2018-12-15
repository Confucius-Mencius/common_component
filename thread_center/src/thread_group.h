#ifndef THREAD_CENTER_SRC_THREAD_GROUP_H_
#define THREAD_CENTER_SRC_THREAD_GROUP_H_

#include <vector>
#include "mem_util.h"
#include "thread.h"

namespace thread_center
{
class ThreadCenter;

class ThreadGroup : public ThreadGroupInterface
{
    CREATE_FUNC(ThreadGroup)

public:
    ThreadGroup();
    virtual ~ThreadGroup();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ThreadGroupInterface /////////////////////////
    ThreadInterface* CreateThread(const ThreadCtx* thread_ctx) override;

    int GetThreadCount() const override
    {
        return (int) thread_vec_.size();
    }

    ThreadInterface* GetThread(int thread_idx) const override;

    int Start() override;
    void Join() override;

    void NotifyStop() override;
    void NotifyReload() override;

    bool CanExit() const override;
    void NotifyExit() override;

    int PushTaskToThread(Task* task, int thread_idx) override;

public:
    void SetThreadCenter(ThreadCenter* thread_center)
    {
        thread_center_ = thread_center;
    }

private:
    ThreadCenter* thread_center_;

    typedef std::vector<Thread*> ThreadVec;
    ThreadVec thread_vec_;
};
} // namespace thread_center

#endif // THREAD_CENTER_SRC_THREAD_GROUP_H_
