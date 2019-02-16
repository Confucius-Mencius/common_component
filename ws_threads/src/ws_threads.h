#ifndef WS_THREADS_SRC_WS_THREADS_H_
#define WS_THREADS_SRC_WS_THREADS_H_

#include "ws_controller.h"
#include "ws_threads_interface.h"

namespace ws
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
    void SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_groups) override;
    ThreadGroupInterface* GetWSThreadGroup() const override;

private:
    ThreadsCtx threads_ctx_;
    ThreadGroupInterface* ws_thread_group_;
    RelatedThreadGroups related_thread_groups_;
    WSController ws_controller_;
};
}

#endif // WS_THREADS_SRC_WS_THREADS_H_
