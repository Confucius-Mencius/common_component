#ifndef HTTP_WS_THREADS_INC_HTTP_WS_THREADS_INTERFACE_H_
#define HTTP_WS_THREADS_INC_HTTP_WS_THREADS_INTERFACE_H_

#include "raw_tcp_threads_interface.h"

class ConfCenterInterface;

namespace global
{
class LogicInterface;
}

namespace app_frame
{
class ConfMgrInterface;
}

namespace tcp
{
namespace http_ws
{
class ThreadsInterface : public ModuleInterface
{
public:
    virtual ~ThreadsInterface()
    {
    }

    virtual int CreateThreadGroup(const char* name_prefix) = 0;
    virtual void SetRelatedThreadGroups(const tcp::RelatedThreadGroups* related_thread_groups) = 0;
    virtual ThreadGroupInterface* GetTCPThreadGroup() const = 0;
};
}
}

#endif // HTTP_WS_THREADS_INC_HTTP_WS_THREADS_INTERFACE_H_
