#ifndef PROTO_TCP_THREADS_INC_PROTO_TCP_THREADS_INTERFACE_H_
#define PROTO_TCP_THREADS_INC_PROTO_TCP_THREADS_INTERFACE_H_

#include "tcp_threads_interface.h"

namespace tcp
{
namespace proto
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

#endif // PROTO_TCP_THREADS_INC_PROTO_TCP_THREADS_INTERFACE_H_
