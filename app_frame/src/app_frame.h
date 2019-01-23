#ifndef APP_FRAME_SRC_APP_FRAME_H_
#define APP_FRAME_SRC_APP_FRAME_H_

#include "app_frame_interface.h"
#include "conf_mgr.h"
#include "module_loader.h"
#include "tcp_threads_interface.h"

namespace app_frame
{
class AppFrame : public AppFrameInterface
{
public:
    AppFrame();
    virtual ~AppFrame();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// AppFrameInterface /////////////////////////
    int NotifyStop() override;
    int NotifyReload(bool changed) override;

    bool CanExit() const override;
    int NotifyExit() override;

private:
    int LoadAndCheckConf();
    int GetDate() const;
//    int LoadGlobalThread();
//    int LoadWorkThreads();
//    int LoadBurdenThreads();
    int LoadTCPThreads();
//    int LoadHttpThreads();
//    int LoadUdpThreads();
//    int LoadRawTcpThreads();
    int CreateAllThreads();
    void SetThreadsRelationship();

private:
    AppFrameCtx app_frame_ctx_;
    ConfMgr conf_mgr_;
    int release_free_mem_date_;

    int app_frame_threads_count_;
//    ModuleLoader global_thread_loader_;
//    global::ThreadsInterface* global_threads_;
//    ModuleLoader work_threads_loader_;
//    work::ThreadsInterface* work_threads_;
//    ModuleLoader burden_threads_loader_;
//    burden::ThreadsInterface* burden_threads_;
    ModuleLoader tcp_threads_loader_;
    tcp::ThreadsInterface* tcp_threads_;
//    ModuleLoader http_threads_loader_;
//    http::ThreadsInterface* http_threads_;
//    ModuleLoader udp_threads_loader_;
//    udp::ThreadsInterface* udp_threads_;
//    ModuleLoader raw_tcp_threads_loader_;
//    tcp::ThreadsInterface* raw_tcp_threads_;
};
}

#endif // APP_FRAME_SRC_APP_FRAME_H_
