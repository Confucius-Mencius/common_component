#ifndef WEB_THREADS_INC_WEB_THREADS_INTERFACE_H_
#define WEB_THREADS_INC_WEB_THREADS_INTERFACE_H_

#include "tcp_threads_interface.h"

namespace tcp
{
namespace web
{
class ThreadsInterface : public ModuleInterface
{
public:
    virtual ~ThreadsInterface()
    {
    }

    virtual int CreateThreadGroup() = 0;
    virtual void SetRelatedThreadGroups(const tcp::RelatedThreadGroups* related_thread_groups) = 0;
    virtual ThreadGroupInterface* GetTCPThreadGroup() const = 0;
};
}
}

#endif // WEB_THREADS_INC_WEB_THREADS_INTERFACE_H_
