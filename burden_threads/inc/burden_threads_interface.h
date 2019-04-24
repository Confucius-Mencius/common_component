#ifndef BURDEN_THREADS_INC_BURDEN_THREADS_INTERFACE_H_
#define BURDEN_THREADS_INC_BURDEN_THREADS_INTERFACE_H_

#include "work_threads_interface.h"

class ConfCenterInterface;

namespace global
{
class LogicInterface;
}

namespace app_frame
{
class ConfMgrInterface;
}

namespace burden
{
class ThreadsInterface : public ModuleInterface
{
public:
    virtual ~ThreadsInterface()
    {
    }

    virtual int CreateThreadGroup() = 0;
    virtual void SetRelatedThreadGroups(const work::RelatedThreadGroups* related_thread_groups) = 0;
    virtual ThreadGroupInterface* GetBurdenThreadGroup() const = 0;
};
}

#endif // BURDEN_THREADS_INC_BURDEN_THREADS_INTERFACE_H_
