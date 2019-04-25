#ifndef HTTP_WS_THREADS_SRC_TCP_THREADS_H_
#define HTTP_WS_THREADS_SRC_TCP_THREADS_H_

#include "http_ws_threads_interface.h"
#include "module_loader.h"
#include "tcp_logic/http_ws_logic_args.h"

namespace tcp
{
namespace http_ws
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
    void SetRelatedThreadGroups(const tcp::RelatedThreadGroups* related_thread_groups) override;
    ThreadGroupInterface* GetTCPThreadGroup() const override;

private:
    int LoadTCPThreads();

private:
    ThreadsCtx threads_ctx_;
    tcp::RelatedThreadGroups related_thread_groups_;

    ModuleLoader tcp_threads_loader_;
    tcp::ThreadsInterface* tcp_threads_;

    tcp::HTTPWSLogicArgs http_ws_logic_args_;
};
}
}

#endif // HTTP_WS_THREADS_SRC_TCP_THREADS_H_
