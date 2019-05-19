#ifndef UDP_THREADS_SRC_UDP_THREADS_H_
#define UDP_THREADS_SRC_UDP_THREADS_H_

#include "module_loader.h"
#include "udp_thread_sink.h"
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
    int CreateThreadGroup() override;
    ThreadGroupInterface* GetUdpThreadGroup() const override;
    void SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group) override;

private:
    int CreateUdpThreads();

private:
    ThreadsCtx threads_ctx_;
    ThreadGroupInterface* udp_thread_group_;

    typedef std::vector<ThreadInterface*> UdpThreadVec;
    UdpThreadVec udp_thread_vec_;

    typedef std::vector<ThreadSink*> UdpThreadSinkVec;
    UdpThreadSinkVec udp_thread_sink_vec_;

    RelatedThreadGroup related_thread_group_;
};
}

#endif // UDP_THREADS_SRC_UDP_THREADS_H_
