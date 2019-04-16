#ifndef UDP_THREADS_SRC_UDP_THREADS_H_
#define UDP_THREADS_SRC_UDP_THREADS_H_

#include "module_loader.h"
#include "thread_sink.h"
#include "udp_threads_interface.h"

namespace udp
{
class Threads : public ThreadsInterface
{
public:
    Threads();
    virtual ~Threads();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ThreadsInterface /////////////////////////
    int CreateThreadGroup(const char* name_prefix) override;
    void SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_groups) override;
    ThreadGroupInterface* GetUDPThreadGroup() const override;

private:
    ThreadsCtx threads_ctx_;
    RelatedThreadGroups related_thread_groups_;
    ThreadGroupInterface* udp_thread_group_;
};
}

#endif // UDP_THREADS_SRC_UDP_THREADS_H_
