#ifndef THREAD_CENTER_SRC_THREAD_CENTER_H_
#define THREAD_CENTER_SRC_THREAD_CENTER_H_

#include <set>
#include "thread_group.h"

namespace thread_center
{
class ThreadCenter : public ThreadCenterInterface
{
public:
    ThreadCenter();
    virtual ~ThreadCenter();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ThreadCenterInterface /////////////////////////
    ThreadGroupInterface* CreateThreadGroup(const ThreadGroupCtx* ctx) override;

public:
    void RemoveThreadGroup(ThreadGroup* thread_group)
    {
        thread_group_set_.erase(thread_group);
    }

private:
    typedef std::set<ThreadGroup*> ThreadGroupSet;
    ThreadGroupSet thread_group_set_;
};
} // namespace thread_center

#endif // THREAD_CENTER_SRC_THREAD_CENTER_H_
