#ifndef TCP_THREADS_SRC_TCP_THREADS_H_
#define TCP_THREADS_SRC_TCP_THREADS_H_

#include "listen_thread_sink.h"
#include "module_loader.h"
#include "tcp_thread_sink.h"
#include "tcp_threads_interface.h"

namespace tcp
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
    ThreadGroupInterface* GetListenThreadGroup() const override;
    ThreadGroupInterface* GetTcpThreadGroup() const override;
    void SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group) override;

private:
    int CreateListenThread();
    int CreateTcpThreads();

private:
    ThreadsCtx threads_ctx_;
    ThreadGroupInterface* listen_thread_group_;
    ThreadGroupInterface* tcp_thread_group_;

    ThreadInterface* listen_thread_;
    ListenThreadSink* listen_thread_sink_;

    typedef std::vector<ThreadInterface*> TcpThreadVec;
    TcpThreadVec tcp_thread_vec_;

    typedef std::vector<ThreadSink*> TcpThreadSinkVec;
    TcpThreadSinkVec tcp_thread_sink_vec_;

    RelatedThreadGroup related_thread_group_;
};
}

#endif // TCP_THREADS_SRC_TCP_THREADS_H_
