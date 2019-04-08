#ifndef APP_FRAME_SRC_APP_FRAME_H_
#define APP_FRAME_SRC_APP_FRAME_H_

#include "app_frame_interface.h"
#include "conf_mgr.h"
#include "module_loader.h"
#include "global_threads_interface.h"
#include "work_threads_interface.h"
#include "burden_threads_interface.h"
#include "raw_tcp_threads_interface.h"
#include "proto_tcp_threads_interface.h"
#include "http_ws_threads_interface.h"

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
    int NotifyReload() override;

    bool CanExit() const override;
    int NotifyExitAndJoin() override;

private:
    int LoadAndCheckConf();

    inline int GetDate() const
    {
        const time_t now = time(nullptr);
        tm* ltm = localtime(&now);

        return (1900 + ltm->tm_year) * 10000 + (1 + ltm->tm_mon) * 100 + ltm->tm_mday;
    }

    int LoadGlobalThread();
    int LoadWorkThreads();
    int LoadBurdenThreads();
    int LoadRawTCPThreads();
    int LoadProtoTCPThreads();

    int LoadHTTPWSThreads();
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

    ModuleLoader global_threads_loader_;
    global::ThreadsInterface* global_threads_;

    ModuleLoader work_threads_loader_;
    work::ThreadsInterface* work_threads_;

    ModuleLoader burden_threads_loader_;
    burden::ThreadsInterface* burden_threads_;

    ModuleLoader raw_tcp_threads_loader_;
    tcp::raw::ThreadsInterface* raw_tcp_threads_;

    ModuleLoader proto_tcp_threads_loader_;
    tcp::proto::ThreadsInterface* proto_tcp_threads_;

    ModuleLoader http_ws_threads_loader_;
    tcp::http_ws::ThreadsInterface* http_ws_threads_;

//    ModuleLoader http_threads_loader_;
//    http::ThreadsInterface* http_threads_;
//    ModuleLoader udp_threads_loader_;
//    udp::ThreadsInterface* udp_threads_;
//    ModuleLoader raw_tcp_threads_loader_;
//    tcp::ThreadsInterface* raw_tcp_threads_;
};
}

#endif // APP_FRAME_SRC_APP_FRAME_H_
