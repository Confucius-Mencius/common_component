#include "app_frame.h"
#include <unistd.h>

#if defined(NDEBUG)
#include <gperftools/profiler.h>
#include <gperftools/heap-profiler.h>
#include <gperftools/malloc_extension.h>
#endif

#include "file_util.h"
#include "mem_util.h"
#include "openssl_util.h"
#include "threads_sync_facility.h"
#include "version.h"

namespace app_frame
{
AppFrame::AppFrame() : app_frame_ctx_(), conf_mgr_(), global_threads_loader_(),
    raw_tcp_threads_loader_(), proto_tcp_threads_loader_()
{
    release_free_mem_date_ = 0;
    app_frame_threads_count_ = 0;
    global_threads_ = nullptr;
    raw_tcp_threads_ = nullptr;
    proto_tcp_threads_ = nullptr;
//    ws_threads_ = nullptr;
}

AppFrame::~AppFrame()
{
}

const char* AppFrame::GetVersion() const
{
    return APP_FRAME_APP_FRAME_VERSION;
}

const char* AppFrame::GetLastErrMsg() const
{
    return NULL;
}

void AppFrame::Release()
{
    SAFE_RELEASE_MODULE(proto_tcp_threads_, proto_tcp_threads_loader_);
    SAFE_RELEASE_MODULE(raw_tcp_threads_, raw_tcp_threads_loader_);
    SAFE_RELEASE_MODULE(global_threads_, global_threads_loader_);

#if defined(NDEBUG)
    if (conf_mgr_.EnableCPUProfiling())
    {
        ProfilerStop();
    }

    if (conf_mgr_.EnableHeapProfiling())
    {
        HeapProfilerStop();
    }
#endif

    delete this;
}

int AppFrame::Initialize(const void* ctx)
{
    if (nullptr == ctx)
    {
        return -1;
    }

    app_frame_ctx_ = *(static_cast<const AppFrameCtx*>(ctx));
    srand(time(NULL));
    // OpenSSLInitialize(); // TODO 初始化openssl，保证只有一次调用 参考libwebsockets中的方法

    if (pthread_mutex_init(&g_app_frame_threads_sync_mutex, NULL) != 0)
    {
        const int err = errno;
        LOG_ERROR("pthread_mutex_init failed, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (pthread_cond_init(&g_app_frame_threads_sync_cond, NULL) != 0)
    {
        const int err = errno;
        LOG_ERROR("pthread_cond_init failed, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (LoadAndCheckConf() != 0)
    {
        return -1;
    }

#if defined(NDEBUG)
    if (conf_mgr_.EnableCPUProfiling())
    {
        LOG_ALWAYS("enable cpu profiling");
        ProfilerStart("cpu_profiling.prof"); // 程序启动目录下
    }

    if (conf_mgr_.EnableHeapProfiling())
    {
        LOG_ALWAYS("enable heap profiling");
        HeapProfilerStart("mem_profiling"); // prefix，程序启动目录下
    }

    LOG_DEBUG("before set, tcmalloc free mem release rate: " << MallocExtension::instance()->GetMemoryReleaseRate());
    MallocExtension::instance()->SetMemoryReleaseRate(5);
    LOG_DEBUG("after set, tcmalloc free mem release rate: " << MallocExtension::instance()->GetMemoryReleaseRate());
#endif

    if (LoadGlobalThread() != 0)
    {
        return -1;
    }

//    if (LoadWorkThreads() != 0)
//    {
//        return -1;
//    }

//    if (LoadBurdenThreads() != 0)
//    {
//        return -1;
//    }


    if (LoadRawTCPThreads() != 0)
    {
        return -1;
    }

    if (LoadProtoTCPThreads() != 0)
    {
        return -1;
    }


//    if (LoadWSThreads() != 0)
//    {
//        return -1;
//    }

//    if (LoadHttpThreads() != 0)
//    {
//        return -1;
//    }

//    if (LoadUdpThreads() != 0)
//    {
//        return -1;
//    }

//    if (LoadRawTcpThreads() != 0)
//    {
//        return -1;
//    }

    if (CreateAllThreads() != 0)
    {
        return -1;
    }

    SetThreadsRelationship();
    return 0;
}

void AppFrame::Finalize()
{
    SAFE_FINALIZE(proto_tcp_threads_);
    SAFE_FINALIZE(raw_tcp_threads_);
    SAFE_FINALIZE(global_threads_);

    conf_mgr_.Finalize();

    pthread_mutex_destroy(&g_app_frame_threads_sync_mutex);
    pthread_cond_destroy(&g_app_frame_threads_sync_cond);

    // OpenSSLFinalize();
}

int AppFrame::Activate()
{
    // activate
    if (SAFE_ACTIVATE_FAILED(global_threads_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(raw_tcp_threads_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(proto_tcp_threads_))
    {
        return -1;
    }

    // ...

    // start
    if (global_threads_ != nullptr)
    {
        if (global_threads_->GetGlobalThreadGroup()->Start() != 0)
        {
            return -1;
        }
    }

    if (raw_tcp_threads_ != nullptr)
    {
        if (raw_tcp_threads_->GetIOThreadGroup()->Start() != 0)
        {
            return -1;
        }

        if (raw_tcp_threads_->GetListenThreadGroup()->Start() != 0)
        {
            return -1;
        }
    }

    if (proto_tcp_threads_ != nullptr)
    {
        if (proto_tcp_threads_->GetIOThreadGroup()->Start() != 0)
        {
            return -1;
        }

        if (proto_tcp_threads_->GetListenThreadGroup()->Start() != 0)
        {
            return -1;
        }
    }

    // 等待所有线程都启动ok
    pthread_mutex_lock(&g_app_frame_threads_sync_mutex);

    while (g_app_frame_threads_count < app_frame_threads_count_)
    {
        pthread_cond_wait(&g_app_frame_threads_sync_cond, &g_app_frame_threads_sync_mutex);
    }

    pthread_mutex_unlock(&g_app_frame_threads_sync_mutex);
    return 0;
}

void AppFrame::Freeze()
{
    SAFE_FREEZE(proto_tcp_threads_);
    SAFE_FREEZE(raw_tcp_threads_);
    SAFE_FREEZE(global_threads_);
}

int AppFrame::NotifyStop()
{
    if (global_threads_ != nullptr && global_threads_->GetGlobalThreadGroup() != nullptr)
    {
        global_threads_->GetGlobalThreadGroup()->NotifyStop();
    }

//    if (work_threads_ != nullptr && work_threads_->GetWorkThreadGroup() != nullptr)
//    {
//        work_threads_->GetWorkThreadGroup()->NotifyStop();
//    }

//    if (burden_threads_ != nullptr && burden_threads_->GetBurdenThreadGroup() != nullptr)
//    {
//        burden_threads_->GetBurdenThreadGroup()->NotifyStop();
//    }

    if (raw_tcp_threads_ != nullptr)
    {
        if (raw_tcp_threads_->GetListenThreadGroup() != nullptr)
        {
            raw_tcp_threads_->GetListenThreadGroup()->NotifyStop();
        }

        if (raw_tcp_threads_->GetIOThreadGroup() != nullptr)
        {
            raw_tcp_threads_->GetIOThreadGroup()->NotifyStop();
        }
    }

    if (proto_tcp_threads_ != nullptr)
    {
        if (proto_tcp_threads_->GetListenThreadGroup() != nullptr)
        {
            proto_tcp_threads_->GetListenThreadGroup()->NotifyStop();
        }

        if (proto_tcp_threads_->GetIOThreadGroup() != nullptr)
        {
            proto_tcp_threads_->GetIOThreadGroup()->NotifyStop();
        }
    }


//    if (ws_threads_ != nullptr)
//    {
//        if (ws_threads_->GetWSThreadGroup() != nullptr)
//        {
//            ws_threads_->GetWSThreadGroup()->NotifyStop();
//        }
//    }

//    if (http_threads_ != nullptr && http_threads_->GetHttpThreadGroup() != nullptr)
//    {
//        http_threads_->GetHttpThreadGroup()->NotifyStop();
//    }

//    if (udp_threads_ != nullptr && udp_threads_->GetUdpThreadGroup() != nullptr)
//    {
//        udp_threads_->GetUdpThreadGroup()->NotifyStop();
//    }

//    if (raw_tcp_threads_ != nullptr)
//    {
//        if (raw_tcp_threads_->GetListenThreadGroup() != nullptr)
//        {
//            raw_tcp_threads_->GetListenThreadGroup()->NotifyStop();
//        }

//        if (raw_tcp_threads_->GetTcpThreadGroup() != nullptr)
//        {
//            raw_tcp_threads_->GetTcpThreadGroup()->NotifyStop();
//        }
//    }

    return 0;
}

int AppFrame::NotifyReload()
{
    conf_mgr_.Reload();

#if defined(NDEBUG)
    if (conf_mgr_.ReleaseFreeMem())
    {
        const int date = GetDate();

        if (release_free_mem_date_ != date)
        {
            LOG_ALWAYS("release free mem right now");
            MallocExtension::instance()->ReleaseFreeMemory();
            release_free_mem_date_ = date;
        }
    }
#endif

    if (global_threads_ != nullptr)
    {
        global_threads_->SetReloadFinish(false);
        global_threads_->GetGlobalThreadGroup()->NotifyReload();

        // 等待global thread完成reload
        while (!global_threads_->ReloadFinished())
        {
            usleep(20); // TODO
        }
    }

//    if (work_threads_ != nullptr)
//    {
//        work_threads_->GetWorkThreadGroup()->NotifyReload();
//    }

//    if (burden_threads_ != nullptr)
//    {
//        burden_threads_->GetBurdenThreadGroup()->NotifyReload();
//    }

    if (raw_tcp_threads_ != nullptr)
    {
        raw_tcp_threads_->GetListenThreadGroup()->NotifyReload();
        raw_tcp_threads_->GetIOThreadGroup()->NotifyReload();
    }

    if (proto_tcp_threads_ != nullptr)
    {
        proto_tcp_threads_->GetListenThreadGroup()->NotifyReload();
        proto_tcp_threads_->GetIOThreadGroup()->NotifyReload();
    }
//    if (ws_threads_ != nullptr)
//    {
//        ws_threads_->GetWSThreadGroup()->NotifyReload();
//    }

//    if (http_threads_ != nullptr)
//    {
//        http_threads_->GetHttpThreadGroup()->NotifyReload();
//    }

//    if (udp_threads_ != nullptr)
//    {
//        udp_threads_->GetUdpThreadGroup()->NotifyReload();
//    }

//    if (raw_tcp_threads_ != nullptr)
//    {
//        raw_tcp_threads_->GetListenThreadGroup()->NotifyReload();
//        raw_tcp_threads_->GetTcpThreadGroup()->NotifyReload();
//    }

    return 0;
}

bool AppFrame::CanExit() const
{
    int can_exit = 1;

    if (global_threads_ != nullptr)
    {
        can_exit &= (global_threads_->GetGlobalThreadGroup()->CanExit() ? 1 : 0);
        LOG_DEBUG("global thread can exit: " << can_exit);
    }

//    if (work_threads_ != nullptr && work_threads_->GetWorkThreadGroup() != nullptr)
//    {
//        can_exit &= (work_threads_->GetWorkThreadGroup()->CanExit() ? 1 : 0);
//        LOG_DEBUG("work threads can exit: " << can_exit);
//    }

//    if (burden_threads_ != nullptr && burden_threads_->GetBurdenThreadGroup() != nullptr)
//    {
//        can_exit &= (burden_threads_->GetBurdenThreadGroup()->CanExit() ? 1 : 0);
//        LOG_DEBUG("burden threads can exit: " << can_exit);
//    }

    if (raw_tcp_threads_ != nullptr)
    {
        can_exit &= (raw_tcp_threads_->GetListenThreadGroup()->CanExit() ? 1 : 0);
        LOG_ALWAYS("raw tcp listen thread can exit: " << can_exit);

        can_exit &= (raw_tcp_threads_->GetIOThreadGroup()->CanExit() ? 1 : 0);
        LOG_ALWAYS("raw tcp threads can exit: " << can_exit);
    }

    if (proto_tcp_threads_ != nullptr)
    {
        can_exit &= (proto_tcp_threads_->GetListenThreadGroup()->CanExit() ? 1 : 0);
        LOG_ALWAYS("proto tcp listen thread can exit: " << can_exit);

        can_exit &= (proto_tcp_threads_->GetIOThreadGroup()->CanExit() ? 1 : 0);
        LOG_ALWAYS("proto tcp threads can exit: " << can_exit);
    }

//    if (ws_threads_ != nullptr)
//    {
//        can_exit &= (ws_threads_->GetWSThreadGroup()->CanExit() ? 1 : 0);
//        LOG_ALWAYS("ws threads can exit: " << can_exit);
//    }

//    if (http_threads_ != nullptr && http_threads_->GetHttpThreadGroup() != nullptr)
//    {
//        can_exit &= (http_threads_->GetHttpThreadGroup()->CanExit() ? 1 : 0);
//        LOG_DEBUG("http threads can exit: " << can_exit);
//    }

//    if (udp_threads_ != nullptr && udp_threads_->GetUdpThreadGroup() != nullptr)
//    {
//        can_exit &= (udp_threads_->GetUdpThreadGroup()->CanExit() ? 1 : 0);
//        LOG_DEBUG("udp threads can exit: " << can_exit);
//    }

//    if (raw_tcp_threads_ != nullptr)
//    {
//        if (raw_tcp_threads_->GetListenThreadGroup() != nullptr)
//        {
//            can_exit &= (raw_tcp_threads_->GetListenThreadGroup()->CanExit() ? 1 : 0);
//            LOG_DEBUG("raw tcp listen thread can exit: " << can_exit);
//        }

//        if (raw_tcp_threads_->GetTcpThreadGroup() != nullptr)
//        {
//            can_exit &= (raw_tcp_threads_->GetTcpThreadGroup()->CanExit() ? 1 : 0);
//            LOG_DEBUG("raw tcp threads can exit: " << can_exit);
//        }
//    }

    return (can_exit != 0);
}

int AppFrame::NotifyExitAndJoin()
{
    if (global_threads_ != nullptr)
    {
        global_threads_->GetGlobalThreadGroup()->NotifyExit();
        global_threads_->GetGlobalThreadGroup()->Join();
    }

//    if (work_threads_ != nullptr && work_threads_->GetWorkThreadGroup() != nullptr)
//    {
//        work_threads_->GetWorkThreadGroup()->NotifyExit();
//    }

//    if (burden_threads_ != nullptr && burden_threads_->GetBurdenThreadGroup() != nullptr)
//    {
//        burden_threads_->GetBurdenThreadGroup()->NotifyExit();
//    }

    if (raw_tcp_threads_ != nullptr)
    {
        raw_tcp_threads_->GetListenThreadGroup()->NotifyExit();
        raw_tcp_threads_->GetListenThreadGroup()->Join();

        raw_tcp_threads_->GetIOThreadGroup()->NotifyExit();
        raw_tcp_threads_->GetIOThreadGroup()->Join();
    }

    if (proto_tcp_threads_ != nullptr)
    {
        proto_tcp_threads_->GetListenThreadGroup()->NotifyExit();
        proto_tcp_threads_->GetListenThreadGroup()->Join();

        proto_tcp_threads_->GetIOThreadGroup()->NotifyExit();
        proto_tcp_threads_->GetIOThreadGroup()->Join();
    }

//    if (ws_threads_ != nullptr)
//    {
//        ws_threads_->GetWSThreadGroup()->NotifyExit();
//        ws_threads_->GetWSThreadGroup()->Join();
//    }

//    if (http_threads_ != nullptr && http_threads_->GetHttpThreadGroup() != nullptr)
//    {
//        http_threads_->GetHttpThreadGroup()->NotifyExit();
//    }

//    if (udp_threads_ != nullptr && udp_threads_->GetUdpThreadGroup() != nullptr)
//    {
//        udp_threads_->GetUdpThreadGroup()->NotifyExit();
//    }

//    if (raw_tcp_threads_ != nullptr)
//    {
//        if (raw_tcp_threads_->GetListenThreadGroup() != nullptr)
//        {
//            raw_tcp_threads_->GetListenThreadGroup()->NotifyExit();
//        }

//        if (raw_tcp_threads_->GetTcpThreadGroup() != nullptr)
//        {
//            raw_tcp_threads_->GetTcpThreadGroup()->NotifyExit();
//        }
//    }

    return 0;
}

int AppFrame::LoadAndCheckConf()
{
    if (conf_mgr_.Initialize(app_frame_ctx_.conf_center) != 0)
    {
        return -1;
    }

    /**
     * 1，通过检查是否配置有监听端口来确定服务是否存在（tcp、udp、http/https/ws/wss）
     * 2，当服务存在时，检查相应的线程数配置是否正确；没有线程数配置的（如global logic），通过检查是否配置有logic so间接确定线程数
     * 3，当线程数>0时，对应的logic so配置是否正确（必配、选配、组之间的可选关系）
     */

    ////////////////////////////////////////////////////////////////////////////////
    // global thread
    if (conf_mgr_.GetGlobalLogicSo().length() > 0)
    {
        LOG_ALWAYS("global thread count: 1");
        ++app_frame_threads_count_;
    }

    bool raw_tcp_exist = false;
    bool proto_tcp_exist = false;

//    bool http_exist = false;
//    bool udp_exist = false;
//    bool raw_tcp_exist = false;

    ////////////////////////////////////////////////////////////////////////////////
    // raw tcp
    if (conf_mgr_.GetRawTCPAddr().length() > 0)
    {
        raw_tcp_exist = true;

        LOG_ALWAYS("raw tcp listen thread count: 1");
        ++app_frame_threads_count_; // listen thread

        if (0 == conf_mgr_.GetRawTCPThreadCount())
        {
            LOG_ERROR("there must be at least one raw tcp thread");
            return -1;
        }

        LOG_ALWAYS("raw tcp thread count: " << conf_mgr_.GetRawTCPThreadCount());
        app_frame_threads_count_ += conf_mgr_.GetRawTCPThreadCount();

        if (0 == conf_mgr_.GetWorkThreadCount())
        {
            // 当没有work thread时，io logic group必须存在
            if (0 == conf_mgr_.GetRawTCPLogicSoGroup().size())
            {
                LOG_ERROR("there is no work thread, so there must be at least one raw tcp logic so");
                return -1;
            }
        }
    }

    if (conf_mgr_.GetProtoTCPAddr().length() > 0)
    {
        proto_tcp_exist = true;

        LOG_ALWAYS("proto tcp listen thread count: 1");
        ++app_frame_threads_count_; // listen thread

        if (0 == conf_mgr_.GetProtoTCPThreadCount())
        {
            LOG_ERROR("there must be at least one proto tcp thread");
            return -1;
        }

        LOG_ALWAYS("proto tcp thread count: " << conf_mgr_.GetProtoTCPThreadCount());
        app_frame_threads_count_ += conf_mgr_.GetProtoTCPThreadCount();

        if (0 == conf_mgr_.GetWorkThreadCount())
        {
            // 当没有work thread时，io logic group必须存在
            if (0 == conf_mgr_.GetProtoTCPLogicSoGroup().size())
            {
                LOG_ERROR("there is no work thread, so there must be at least one proto tcp logic so");
                return -1;
            }
        }
    }

    // ws
//    if (conf_mgr_.GetWSIface().length() > 0)
//    {
//        ws_exist = true; // 有ws

//        if (0 == conf_mgr_.GetWSThreadCount())
//        {
//            LOG_ERROR("there must be at least one ws thread");
//            return -1;
//        }

//        LOG_ALWAYS("ws thread count: " << conf_mgr_.GetWSThreadCount());
//        app_frame_threads_count_ += conf_mgr_.GetWSThreadCount();

//        if (0 == conf_mgr_.GetWorkThreadCount())
//        {
//            // 当没有work thread时，io logic group必须存在
//            if (0 == conf_mgr_.GetWSLogicSoGroup().size())
//            {
//                LOG_ERROR("there is no work thread, so there must be at least one ws logic so");
//                return -1;
//            }
//        }
//    }

    // TODO

//    ////////////////////////////////////////////////////////////////////////////////
//    // http/https
//    if (conf_mgr_.GetHttpAddrPort().length() > 0 || conf_mgr_.GetHttpsAddrPort().length() > 0)
//    {
//        http_exist = true; // 有http/https

//        if (0 == conf_mgr_.GetHttpThreadCount())
//        {
//            LOG_ERROR("there must be at least one http/https thread");
//            return -1;
//        }

//        LOG_INFO("http/https thread count: " << conf_mgr_.GetHttpThreadCount());
//        app_frame_threads_count_ += conf_mgr_.GetHttpThreadCount();

//        // 当http thread存在时，http local logic是可选的，http logic group必须存在
//        if (0 == conf_mgr_.GetHttpLogicSoGroup().size())
//        {
//            LOG_ERROR("there must be at least one http logic so");
//            return -1;
//        }
//    }

//    ////////////////////////////////////////////////////////////////////////////////
//    // udp
//    if (conf_mgr_.GetUdpAddrPort().length() > 0)
//    {
//        udp_exist = true; // 有udp

//        if (0 == conf_mgr_.GetUdpThreadCount())
//        {
//            LOG_ERROR("there must be at least one udp thread");
//            return -1;
//        }

//        LOG_INFO("udp thread count: " << conf_mgr_.GetUdpThreadCount());
//        app_frame_threads_count_ += conf_mgr_.GetUdpThreadCount();

//        if (0 == conf_mgr_.GetWorkThreadCount())
//        {
//            // 当没有work thread时，io logic group必须存在
//            if (0 == conf_mgr_.GetUdpLogicSoGroup().size())
//            {
//                LOG_ERROR("there is no work thread, so there must be at least one udp logic so");
//                return -1;
//            }
//        }
//    }

//    ////////////////////////////////////////////////////////////////////////////////
//    // raw tcp
//    if (conf_mgr_.GetRawTcpAddrPort().length() > 0)
//    {
//        raw_tcp_exist = true; // 有raw tcp

//        LOG_INFO("raw tcp listen thread count: 1");
//        ++app_frame_threads_count_; // listen thread

//        if (0 == conf_mgr_.GetRawTcpThreadCount())
//        {
//            LOG_ERROR("there must be at least one raw tcp thread");
//            return -1;
//        }

//        LOG_INFO("raw tcp thread count: " << conf_mgr_.GetRawTcpThreadCount());
//        app_frame_threads_count_ += conf_mgr_.GetRawTcpThreadCount();

//        if (0 == conf_mgr_.GetWorkThreadCount())
//        {
//            // 当没有work thread时，io logic group必须存在
//            if (0 == conf_mgr_.GetRawTcpLogicSoGroup().size())
//            {
//                LOG_ERROR("there is no work thread, so there must be at least one raw tcp logic so");
//                return -1;
//            }
//        }
//    }

//    ////////////////////////////////////////////////////////////////////////////////
//    // work/burden thread，work/burden logic检查

//    // 不允许没有work thread的情况下出现burden thread
//    if (0 == conf_mgr_.GetWorkThreadCount() && conf_mgr_.GetBurdenThreadCount() > 0)
//    {
//        LOG_ERROR("no work thread but burden thread count is " << conf_mgr_.GetBurdenThreadCount());
//        return -1;
//    }

//    if (conf_mgr_.GetBurdenThreadCount() > 0)
//    {
//        LOG_INFO("burden thread count: " << conf_mgr_.GetBurdenThreadCount());
//        app_frame_threads_count_ += conf_mgr_.GetBurdenThreadCount();

//        // 当burden thread存在时，burden local logic是可选的，burden logic group必须存在
//        if (0 == conf_mgr_.GetBurdenLogicSoGroup().size())
//        {
//            LOG_ERROR("there must be at least one burden logic so");
//            return -1;
//        }
//    }

//    if (conf_mgr_.GetWorkThreadCount() > 0)
//    {
//        LOG_INFO("work thread count: " << conf_mgr_.GetWorkThreadCount());
//        app_frame_threads_count_ += conf_mgr_.GetWorkThreadCount();

//        if (0 == conf_mgr_.GetBurdenThreadCount() && 0 == conf_mgr_.GetTcpLogicSoGroup().size()
//                && 0 == conf_mgr_.GetUdpLogicSoGroup().size() && 0 == conf_mgr_.GetRawTcpLogicSoGroup().size())
//        {
//            // 当没有burden thread和io logic group时，work logic group必须存在，work local logic是可选的
//            if (0 == conf_mgr_.GetWorkLogicSoGroup().size())
//            {
//                LOG_ERROR("there is no burden thread and io logic group, so there must be at least one work logic so");
//                return -1;
//            }
//        }
//    }

    // tcp、http、udp可以同时存在，也可以只有一个存在，不能都不存在
//    if (!tcp_exist && !http_exist && !udp_exist && !raw_tcp_exist)
    if (!raw_tcp_exist && !proto_tcp_exist)
    {
        LOG_ERROR("there must be one raw tcp or proto tcp or ws or udp io module");
        return -1;
    }

    return 0;
}

int AppFrame::GetDate() const
{
    const time_t now = time(NULL);
    tm* ltm = localtime(&now);

    return (1900 + ltm->tm_year) * 10000 + (1 + ltm->tm_mon) * 100 + ltm->tm_mday;
}

int AppFrame::LoadGlobalThread()
{
    if (0 == conf_mgr_.GetGlobalLogicSo().length())
    {
        return 0;
    }

    char GLOBAL_THREAD_SO_PATH[MAX_PATH_LEN] = "";
    StrPrintf(GLOBAL_THREAD_SO_PATH, sizeof(GLOBAL_THREAD_SO_PATH), "%s/libglobal_thread.so",
              app_frame_ctx_.common_component_dir);

    if (global_threads_loader_.Load(GLOBAL_THREAD_SO_PATH) != 0)
    {
        LOG_ERROR(global_threads_loader_.GetLastErrMsg());
        return -1;
    }

    global_threads_ = static_cast<global::ThreadsInterface*>(global_threads_loader_.GetModuleInterface());
    if (nullptr == global_threads_)
    {
        LOG_ERROR(global_threads_loader_.GetLastErrMsg());
        return -1;
    }

    global::ThreadsCtx threads_ctx;
    threads_ctx.argc = app_frame_ctx_.argc;
    threads_ctx.argv = app_frame_ctx_.argv;
    threads_ctx.common_component_dir = app_frame_ctx_.common_component_dir;
    threads_ctx.cur_working_dir = app_frame_ctx_.cur_working_dir;
    threads_ctx.app_name = app_frame_ctx_.app_name;
    threads_ctx.conf_center = app_frame_ctx_.conf_center;
    threads_ctx.thread_center = app_frame_ctx_.thread_center;
    threads_ctx.app_frame_conf_mgr = &conf_mgr_;
    threads_ctx.app_frame_threads_count = &g_app_frame_threads_count;
    threads_ctx.app_frame_threads_sync_mutex = &g_app_frame_threads_sync_mutex;
    threads_ctx.app_frame_threads_sync_cond = &g_app_frame_threads_sync_cond;

    if (global_threads_->Initialize(&threads_ctx) != 0)
    {
        return -1;
    }

    return 0;
}

//int AppFrame::LoadWorkThreads()
//{
//    if (0 == conf_mgr_.GetWorkThreadCount())
//    {
//        return 0;
//    }

//    char WORK_THREADS_SO_PATH[MAX_PATH_LEN] = "";
//    StrPrintf(WORK_THREADS_SO_PATH, sizeof(WORK_THREADS_SO_PATH), "%s/libwork_threads.so",
//              app_frame_ctx_.common_component_dir);

//    if (work_threads_loader_.Load(WORK_THREADS_SO_PATH) != 0)
//    {
//        LOG_ERROR(work_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    work_threads_ = (work::ThreadsInterface*) work_threads_loader_.GetModuleInterface();
//    if (nullptr == work_threads_)
//    {
//        LOG_ERROR(work_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    work::ThreadsCtx threads_ctx;
//    threads_ctx.argc = app_frame_ctx_.argc;
//    threads_ctx.argv = app_frame_ctx_.argv;
//    threads_ctx.common_component_dir = app_frame_ctx_.common_component_dir;
//    threads_ctx.cur_work_dir = app_frame_ctx_.cur_working_dir;
//    threads_ctx.app_name = app_frame_ctx_.app_name;
//    threads_ctx.conf_center = app_frame_ctx_.conf_center;
//    threads_ctx.msg_codec_center = app_frame_ctx_.msg_codec_center;
//    threads_ctx.thread_center = app_frame_ctx_.thread_center;
//    threads_ctx.conf_mgr = &conf_mgr_;
//    threads_ctx.frame_threads_count = &g_frame_threads_count;
//    threads_ctx.frame_threads_mutex = &g_frame_threads_mutex;
//    threads_ctx.frame_threads_cond = &g_frame_threads_cond;

//    if (work_threads_->Initialize(&threads_ctx) != 0)
//    {
//        return -1;
//    }

//    return 0;
//}

//int AppFrame::LoadBurdenThreads()
//{
//    if (0 == conf_mgr_.GetWorkThreadCount() || 0 == conf_mgr_.GetBurdenThreadCount())
//    {
//        return 0;
//    }

//    char BURDEN_THREADS_SO_PATH[MAX_PATH_LEN] = "";
//    StrPrintf(BURDEN_THREADS_SO_PATH, sizeof(BURDEN_THREADS_SO_PATH), "%s/libburden_threads.so",
//              app_frame_ctx_.common_component_dir);

//    if (burden_threads_loader_.Load(BURDEN_THREADS_SO_PATH) != 0)
//    {
//        LOG_ERROR(burden_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    burden_threads_ = (burden::ThreadsInterface*) burden_threads_loader_.GetModuleInterface();
//    if (nullptr == burden_threads_)
//    {
//        LOG_ERROR(burden_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    burden::ThreadsCtx threads_ctx;
//    threads_ctx.argc = app_frame_ctx_.argc;
//    threads_ctx.argv = app_frame_ctx_.argv;
//    threads_ctx.common_component_dir = app_frame_ctx_.common_component_dir;
//    threads_ctx.cur_work_dir = app_frame_ctx_.cur_working_dir;
//    threads_ctx.app_name = app_frame_ctx_.app_name;
//    threads_ctx.conf_center = app_frame_ctx_.conf_center;
//    threads_ctx.msg_codec_center = app_frame_ctx_.msg_codec_center;
//    threads_ctx.thread_center = app_frame_ctx_.thread_center;
//    threads_ctx.conf_mgr = &conf_mgr_;
//    threads_ctx.frame_threads_count = &g_frame_threads_count;
//    threads_ctx.frame_threads_mutex = &g_frame_threads_mutex;
//    threads_ctx.frame_threads_cond = &g_frame_threads_cond;

//    if (burden_threads_->Initialize(&threads_ctx) != 0)
//    {
//        return -1;
//    }

//    return 0;
//}

int AppFrame::LoadRawTCPThreads()
{
    if (0 == conf_mgr_.GetRawTCPThreadCount())
    {
        return 0;
    }

    char RAW_TCP_THREADS_SO_PATH[MAX_PATH_LEN] = "";
    StrPrintf(RAW_TCP_THREADS_SO_PATH, sizeof(RAW_TCP_THREADS_SO_PATH), "%s/libraw_tcp_threads.so",
              app_frame_ctx_.common_component_dir);

    if (raw_tcp_threads_loader_.Load(RAW_TCP_THREADS_SO_PATH) != 0)
    {
        LOG_ERROR(raw_tcp_threads_loader_.GetLastErrMsg());
        return -1;
    }

    raw_tcp_threads_ = static_cast<tcp::raw::ThreadsInterface*>(raw_tcp_threads_loader_.GetModuleInterface());
    if (nullptr == raw_tcp_threads_)
    {
        LOG_ERROR(raw_tcp_threads_loader_.GetLastErrMsg());
        return -1;
    }

    tcp::ThreadsCtx threads_ctx;
    threads_ctx.argc = app_frame_ctx_.argc;
    threads_ctx.argv = app_frame_ctx_.argv;
    threads_ctx.common_component_dir = app_frame_ctx_.common_component_dir;
    threads_ctx.cur_working_dir = app_frame_ctx_.cur_working_dir;
    threads_ctx.app_name = app_frame_ctx_.app_name;
    threads_ctx.conf_center = app_frame_ctx_.conf_center;
    threads_ctx.thread_center = app_frame_ctx_.thread_center;
    threads_ctx.app_frame_conf_mgr = &conf_mgr_;
    threads_ctx.app_frame_threads_count = &g_app_frame_threads_count;
    threads_ctx.app_frame_threads_sync_mutex = &g_app_frame_threads_sync_mutex;
    threads_ctx.app_frame_threads_sync_cond = &g_app_frame_threads_sync_cond;
    threads_ctx.conf.io_type = IO_TYPE_RAW_TCP;
    threads_ctx.conf.addr = conf_mgr_.GetRawTCPAddr();
    threads_ctx.conf.port = conf_mgr_.GetRawTCPPort();
    threads_ctx.conf.thread_count = conf_mgr_.GetRawTCPThreadCount();
    threads_ctx.conf.common_logic_so = conf_mgr_.GetRawTCPCommonLogicSo();
    threads_ctx.conf.logic_so_group = conf_mgr_.GetRawTCPLogicSoGroup();
    threads_ctx.logic_args = nullptr;

    if (raw_tcp_threads_->Initialize(&threads_ctx) != 0)
    {
        return -1;
    }

    return 0;
}

int AppFrame::LoadProtoTCPThreads()
{
    if (0 == conf_mgr_.GetProtoTCPThreadCount())
    {
        return 0;
    }

    char PROTO_TCP_THREADS_SO_PATH[MAX_PATH_LEN] = "";
    StrPrintf(PROTO_TCP_THREADS_SO_PATH, sizeof(PROTO_TCP_THREADS_SO_PATH), "%s/libproto_tcp_threads.so",
              app_frame_ctx_.common_component_dir);

    if (proto_tcp_threads_loader_.Load(PROTO_TCP_THREADS_SO_PATH) != 0)
    {
        LOG_ERROR(proto_tcp_threads_loader_.GetLastErrMsg());
        return -1;
    }

    proto_tcp_threads_ = static_cast<tcp::proto::ThreadsInterface*>(proto_tcp_threads_loader_.GetModuleInterface());
    if (nullptr == proto_tcp_threads_)
    {
        LOG_ERROR(proto_tcp_threads_loader_.GetLastErrMsg());
        return -1;
    }

    tcp::ThreadsCtx threads_ctx;
    threads_ctx.argc = app_frame_ctx_.argc;
    threads_ctx.argv = app_frame_ctx_.argv;
    threads_ctx.common_component_dir = app_frame_ctx_.common_component_dir;
    threads_ctx.cur_working_dir = app_frame_ctx_.cur_working_dir;
    threads_ctx.app_name = app_frame_ctx_.app_name;
    threads_ctx.conf_center = app_frame_ctx_.conf_center;
    threads_ctx.thread_center = app_frame_ctx_.thread_center;
    threads_ctx.app_frame_conf_mgr = &conf_mgr_;
    threads_ctx.app_frame_threads_count = &g_app_frame_threads_count;
    threads_ctx.app_frame_threads_sync_mutex = &g_app_frame_threads_sync_mutex;
    threads_ctx.app_frame_threads_sync_cond = &g_app_frame_threads_sync_cond;
    // conf和logic_args字段由proto tcp threads内部填写

    if (proto_tcp_threads_->Initialize(&threads_ctx) != 0)
    {
        return -1;
    }

    return 0;
}

//int AppFrame::LoadWSThreads()
//{
//    if (0 == conf_mgr_.GetWSThreadCount())
//    {
//        return 0;
//    }

//    char WS_THREADS_SO_PATH[MAX_PATH_LEN] = "";
//    StrPrintf(WS_THREADS_SO_PATH, sizeof(WS_THREADS_SO_PATH), "%s/libws_threads.so",
//              app_frame_ctx_.common_component_dir);

//    if (ws_threads_loader_.Load(WS_THREADS_SO_PATH) != 0)
//    {
//        LOG_ERROR(ws_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    ws_threads_ = static_cast<ws::ThreadsInterface*>(ws_threads_loader_.GetModuleInterface());
//    if (nullptr == ws_threads_)
//    {
//        LOG_ERROR(ws_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    ws::ThreadsCtx threads_ctx;
//    threads_ctx.argc = app_frame_ctx_.argc;
//    threads_ctx.argv = app_frame_ctx_.argv;
//    threads_ctx.common_component_dir = app_frame_ctx_.common_component_dir;
//    threads_ctx.cur_working_dir = app_frame_ctx_.cur_working_dir;
//    threads_ctx.app_name = app_frame_ctx_.app_name;
//    threads_ctx.conf_center = app_frame_ctx_.conf_center;
//    threads_ctx.thread_center = app_frame_ctx_.thread_center;
//    threads_ctx.conf_mgr = &conf_mgr_;
//    threads_ctx.app_frame_threads_count = &g_app_frame_threads_count;
//    threads_ctx.app_frame_threads_sync_mutex = &g_app_frame_threads_sync_mutex;
//    threads_ctx.app_frame_threads_sync_cond = &g_app_frame_threads_sync_cond;

//    if (ws_threads_->Initialize(&threads_ctx) != 0)
//    {
//        return -1;
//    }

//    return 0;
//}

//int AppFrame::LoadHttpThreads()
//{
//    if (0 == conf_mgr_.GetHttpThreadCount())
//    {
//        return 0;
//    }

//    char HTTP_THREADS_SO_PATH[MAX_PATH_LEN] = "";
//    StrPrintf(HTTP_THREADS_SO_PATH, sizeof(HTTP_THREADS_SO_PATH), "%s/libhttp_threads.so",
//              app_frame_ctx_.common_component_dir);

//    if (http_threads_loader_.Load(HTTP_THREADS_SO_PATH) != 0)
//    {
//        LOG_ERROR(http_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    http_threads_ = (http::ThreadsInterface*) http_threads_loader_.GetModuleInterface();
//    if (nullptr == http_threads_)
//    {
//        LOG_ERROR(http_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    http::ThreadsCtx threads_ctx;
//    threads_ctx.argc = app_frame_ctx_.argc;
//    threads_ctx.argv = app_frame_ctx_.argv;
//    threads_ctx.common_component_dir = app_frame_ctx_.common_component_dir;
//    threads_ctx.cur_work_dir = app_frame_ctx_.cur_working_dir;
//    threads_ctx.app_name = app_frame_ctx_.app_name;
//    threads_ctx.conf_center = app_frame_ctx_.conf_center;
//    threads_ctx.msg_codec_center = app_frame_ctx_.msg_codec_center;
//    threads_ctx.thread_center = app_frame_ctx_.thread_center;
//    threads_ctx.conf_mgr = &conf_mgr_;
//    threads_ctx.frame_threads_count = &g_frame_threads_count;
//    threads_ctx.frame_threads_mutex = &g_frame_threads_mutex;
//    threads_ctx.frame_threads_cond = &g_frame_threads_cond;

//    if (http_threads_->Initialize(&threads_ctx) != 0)
//    {
//        return -1;
//    }

//    return 0;
//}

//int AppFrame::LoadUdpThreads()
//{
//    if (0 == conf_mgr_.GetUdpThreadCount())
//    {
//        return 0;
//    }

//    char UDP_THREADS_SO_PATH[MAX_PATH_LEN] = "";
//    StrPrintf(UDP_THREADS_SO_PATH, sizeof(UDP_THREADS_SO_PATH), "%s/libudp_threads.so",
//              app_frame_ctx_.common_component_dir);

//    if (udp_threads_loader_.Load(UDP_THREADS_SO_PATH) != 0)
//    {
//        LOG_ERROR(udp_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    udp_threads_ = (udp::ThreadsInterface*) udp_threads_loader_.GetModuleInterface();
//    if (nullptr == udp_threads_)
//    {
//        LOG_ERROR(udp_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    udp::ThreadsCtx threads_ctx;
//    threads_ctx.argc = app_frame_ctx_.argc;
//    threads_ctx.argv = app_frame_ctx_.argv;
//    threads_ctx.common_component_dir = app_frame_ctx_.common_component_dir;
//    threads_ctx.cur_work_dir = app_frame_ctx_.cur_working_dir;
//    threads_ctx.app_name = app_frame_ctx_.app_name;
//    threads_ctx.conf_center = app_frame_ctx_.conf_center;
//    threads_ctx.msg_codec_center = app_frame_ctx_.msg_codec_center;
//    threads_ctx.thread_center = app_frame_ctx_.thread_center;
//    threads_ctx.conf_mgr = &conf_mgr_;
//    threads_ctx.frame_threads_count = &g_frame_threads_count;
//    threads_ctx.frame_threads_mutex = &g_frame_threads_mutex;
//    threads_ctx.frame_threads_cond = &g_frame_threads_cond;

//    if (udp_threads_->Initialize(&threads_ctx) != 0)
//    {
//        return -1;
//    }

//    return 0;
//}

//int AppFrame::LoadRawTcpThreads()
//{
//    if (0 == conf_mgr_.GetRawTcpThreadCount())
//    {
//        return 0;
//    }

//    char TCP_THREADS_SO_PATH[MAX_PATH_LEN] = "";
//    StrPrintf(TCP_THREADS_SO_PATH, sizeof(TCP_THREADS_SO_PATH), "%s/libtcp_threads.so",
//              app_frame_ctx_.common_component_dir);

//    if (raw_tcp_threads_loader_.Load(TCP_THREADS_SO_PATH) != 0)
//    {
//        LOG_ERROR(raw_tcp_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    raw_tcp_threads_ = (tcp::ThreadsInterface*) raw_tcp_threads_loader_.GetModuleInterface();
//    if (nullptr == raw_tcp_threads_)
//    {
//        LOG_ERROR(raw_tcp_threads_loader_.GetLastErrMsg());
//        return -1;
//    }

//    tcp::ThreadsCtx threads_ctx;
//    threads_ctx.argc = app_frame_ctx_.argc;
//    threads_ctx.argv = app_frame_ctx_.argv;
//    threads_ctx.common_component_dir = app_frame_ctx_.common_component_dir;
//    threads_ctx.cur_work_dir = app_frame_ctx_.cur_working_dir;
//    threads_ctx.app_name = app_frame_ctx_.app_name;
//    threads_ctx.conf_center = app_frame_ctx_.conf_center;
//    threads_ctx.msg_codec_center = app_frame_ctx_.msg_codec_center;
//    threads_ctx.thread_center = app_frame_ctx_.thread_center;
//    threads_ctx.conf_mgr = &conf_mgr_;
//    threads_ctx.frame_threads_count = &g_frame_threads_count;
//    threads_ctx.frame_threads_mutex = &g_frame_threads_mutex;
//    threads_ctx.frame_threads_cond = &g_frame_threads_cond;
//    threads_ctx.raw = true;

//    if (raw_tcp_threads_->Initialize(&threads_ctx) != 0)
//    {
//        return -1;
//    }

//    return 0;
//}

int AppFrame::CreateAllThreads()
{
    LOG_TRACE("AppFrame::CreateAllThreads begin");

    if (conf_mgr_.GetGlobalLogicSo().length() > 0)
    {
        if (global_threads_->CreateThreadGroup() != 0)
        {
            return -1;
        }
    }

//    if (conf_mgr_.GetWorkThreadCount() > 0)
//    {
//        if (work_threads_->CreateThreadGroup() != 0)
//        {
//            return -1;
//        }
//    }

//    if (conf_mgr_.GetBurdenThreadCount() > 0)
//    {
//        if (burden_threads_->CreateThreadGroup() != 0)
//        {
//            return -1;
//        }
//    }

    if (conf_mgr_.GetRawTCPThreadCount() > 0)
    {
        if (raw_tcp_threads_->CreateThreadGroup("raw tcp") != 0)
        {
            return -1;
        }
    }

    if (conf_mgr_.GetProtoTCPThreadCount() > 0)
    {
        if (proto_tcp_threads_->CreateThreadGroup("proto tcp") != 0)
        {
            return -1;
        }
    }

//    if (conf_mgr_.GetWSThreadCount() > 0)
//    {
//        if (ws_threads_->CreateThreadGroup() != 0)
//        {
//            return -1;
//        }
//    }

//    if (conf_mgr_.GetHttpThreadCount() > 0)
//    {
//        if (http_threads_->CreateThreadGroup() != 0)
//        {
//            return -1;
//        }
//    }

//    if (conf_mgr_.GetUdpThreadCount() > 0)
//    {
//        if (udp_threads_->CreateThreadGroup() != 0)
//        {
//            return -1;
//        }
//    }

//    if (conf_mgr_.GetRawTcpThreadCount() > 0)
//    {
//        if (raw_tcp_threads_->CreateThreadGroup() != 0)
//        {
//            return -1;
//        }
//    }

    LOG_TRACE("AppFrame::CreateAllThreads end");
    return 0;
}

void AppFrame::SetThreadsRelationship()
{
    LOG_TRACE("AppFrame::SetThreadsRelationship begin");

    // 各线程组互相访问
    if (global_threads_ != nullptr)
    {
        global::RelatedThreadGroups related_thread_groups;

//        if (burden_threads_ != nullptr)
//        {
//            related_thread_groups.burden_thread_group = burden_threads_->GetBurdenThreadGroup();
//        }

//        if (work_threads_ != nullptr)
//        {
//            related_thread_groups.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

        if (proto_tcp_threads_ != nullptr)
        {
            related_thread_groups.proto_tcp_thread_group = proto_tcp_threads_->GetIOThreadGroup();
        }

        global_threads_->SetRelatedThreadGroups(&related_thread_groups);
    }

//    if (work_threads_ != nullptr)
//    {
//        work::RelatedThreadGroup work_related_thread_group;

//        if (global_threads_ != nullptr)
//        {
//            work_related_thread_group.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            work_related_thread_group.global_logic = global_threads_->GetLogic();
//        }

//        if (burden_threads_ != nullptr)
//        {
//            work_related_thread_group.burden_thread_group = burden_threads_->GetBurdenThreadGroup();
//        }

//        if (tcp_threads_ != nullptr)
//        {
//            work_related_thread_group.tcp_thread_group = tcp_threads_->GetTcpThreadGroup();
//        }

//        if (http_threads_ != nullptr)
//        {
//            work_related_thread_group.http_thread_group = http_threads_->GetHttpThreadGroup();
//        }

//        if (udp_threads_ != nullptr)
//        {
//            work_related_thread_group.udp_thread_group = udp_threads_->GetUdpThreadGroup();
//        }

//        work_threads_->SetRelatedThreadGroup(&work_related_thread_group);
//    }

//    if (burden_threads_ != nullptr)
//    {
//        burden::RelatedThreadGroup burden_related_thread_group;

//        if (global_threads_ != nullptr)
//        {
//            burden_related_thread_group.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            burden_related_thread_group.global_logic = global_threads_->GetLogic();
//        }

//        if (work_threads_ != nullptr)
//        {
//            burden_related_thread_group.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

//        if (tcp_threads_ != nullptr)
//        {
//            burden_related_thread_group.tcp_thread_group = tcp_threads_->GetTcpThreadGroup();
//        }

//        if (http_threads_ != nullptr)
//        {
//            burden_related_thread_group.http_thread_group = http_threads_->GetHttpThreadGroup();
//        }

//        if (udp_threads_ != nullptr)
//        {
//            burden_related_thread_group.udp_thread_group = udp_threads_->GetUdpThreadGroup();
//        }

//        burden_threads_->SetRelatedThreadGroup(&burden_related_thread_group);
//    }

    if (raw_tcp_threads_ != nullptr)
    {
        tcp::RelatedThreadGroups related_thread_groups;

        if (global_threads_ != nullptr)
        {
            related_thread_groups.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
            related_thread_groups.global_logic = global_threads_->GetLogic();
        }

//        if (work_threads_ != nullptr)
//        {
//            tcp_related_thread_groups.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

        raw_tcp_threads_->SetRelatedThreadGroups(&related_thread_groups);
    }

    if (proto_tcp_threads_ != nullptr)
    {
        tcp::RelatedThreadGroups related_thread_groups;

        if (global_threads_ != nullptr)
        {
            related_thread_groups.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
            related_thread_groups.global_logic = global_threads_->GetLogic();
        }

//        if (work_threads_ != nullptr)
//        {
//            tcp_related_thread_groups.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

        proto_tcp_threads_->SetRelatedThreadGroups(&related_thread_groups);
    }

//    if (ws_threads_ != nullptr)
//    {
//        ws::RelatedThreadGroups ws_related_thread_groups;

//        if (global_threads_ != nullptr)
//        {
//            ws_related_thread_groups.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            ws_related_thread_groups.global_logic = global_threads_->GetLogic();
//        }

//        if (work_threads_ != nullptr)
//        {
//            ws_related_thread_groups.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

//        ws_threads_->SetRelatedThreadGroups(&ws_related_thread_groups);
//}

//    if (http_threads_ != nullptr)
//    {
//        http::RelatedThreadGroup http_related_thread_group;

//        if (global_threads_ != nullptr)
//        {
//            http_related_thread_group.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            http_related_thread_group.global_logic = global_threads_->GetLogic();
//        }

//        if (work_threads_ != nullptr)
//        {
//            http_related_thread_group.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

//        http_threads_->SetRelatedThreadGroup(&http_related_thread_group);
//    }

//    if (udp_threads_ != nullptr)
//    {
//        udp::RelatedThreadGroup udp_related_thread_group;

//        if (global_threads_ != nullptr)
//        {
//            udp_related_thread_group.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            udp_related_thread_group.global_logic = global_threads_->GetLogic();
//        }

//        if (work_threads_ != nullptr)
//        {
//            udp_related_thread_group.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

//        udp_threads_->SetRelatedThreadGroup(&udp_related_thread_group);
//    }

//    if (raw_tcp_threads_ != nullptr)
//    {
//        tcp::RelatedThreadGroup tcp_related_thread_group;

//        if (global_threads_ != nullptr)
//        {
//            tcp_related_thread_group.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            tcp_related_thread_group.global_logic = global_threads_->GetLogic();
//        }

//        if (work_threads_ != nullptr)
//        {
//            tcp_related_thread_group.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

//        raw_tcp_threads_->SetRelatedThreadGroup(&tcp_related_thread_group);
//    }

    LOG_TRACE("AppFrame::SetThreadsRelationship end");
}
}
