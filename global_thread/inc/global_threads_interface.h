#ifndef GLOBAL_THREAD_INC_GLOBAL_THREADS_INTERFACE_H_
#define GLOBAL_THREAD_INC_GLOBAL_THREADS_INTERFACE_H_

#include "work_threads_interface.h"

namespace global
{
class LogicInterface;

class ThreadsInterface : public ModuleInterface
{
public:
    virtual ~ThreadsInterface()
    {
    }

    virtual int CreateThreadGroup() = 0;
    virtual void SetRelatedThreadGroups(const work::RelatedThreadGroups* related_thread_groups) = 0;

    virtual ThreadGroupInterface* GetGlobalThreadGroup() const = 0;
    virtual LogicInterface* GetLogic() const = 0; // global logic
};
}

#endif // GLOBAL_THREAD_INC_GLOBAL_THREADS_INTERFACE_H_
