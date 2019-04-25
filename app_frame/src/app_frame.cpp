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
    work_threads_loader_(), burden_threads_loader_(), tcp_threads_loader_(),
    proto_tcp_threads_loader_(), http_ws_threads_loader_()
{
    release_free_mem_date_ = 0;
    app_frame_threads_count_ = 0;
    global_threads_ = nullptr;
    work_threads_ = nullptr;
    burden_threads_ = nullptr;
    tcp_threads_ = nullptr;
    proto_tcp_threads_ = nullptr;
    http_ws_threads_ = nullptr;
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
    return nullptr;
}

void AppFrame::Release()
{
    SAFE_RELEASE_MODULE(http_ws_threads_, http_ws_threads_loader_);
    SAFE_RELEASE_MODULE(proto_tcp_threads_, proto_tcp_threads_loader_);
    SAFE_RELEASE_MODULE(tcp_threads_, tcp_threads_loader_);
    SAFE_RELEASE_MODULE(burden_threads_, burden_threads_loader_);
    SAFE_RELEASE_MODULE(work_threads_, work_threads_loader_);
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
    srand(time(nullptr));
    // OpenSSLInitialize(); // TODO 初始化openssl，保证只有一次调用 参考libwebsockets中的方法

    if (pthread_mutex_init(&g_app_frame_threads_sync_mutex, nullptr) != 0)
    {
        const int err = errno;
        LOG_ERROR("pthread_mutex_init failed, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (pthread_cond_init(&g_app_frame_threads_sync_cond, nullptr) != 0)
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

    if (LoadWorkThreads() != 0)
    {
        return -1;
    }

    if (LoadBurdenThreads() != 0)
    {
        return -1;
    }

    if (LoadTCPThreads() != 0)
    {
        return -1;
    }

    if (LoadProtoTCPThreads() != 0)
    {
        return -1;
    }

    if (LoadHTTPWSThreads() != 0)
    {
        return -1;
    }

//    if (LoadUdpThreads() != 0)
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
    SAFE_FINALIZE(http_ws_threads_);
    SAFE_FINALIZE(proto_tcp_threads_);
    SAFE_FINALIZE(tcp_threads_);
    SAFE_FINALIZE(burden_threads_);
    SAFE_FINALIZE(work_threads_);
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

    if (SAFE_ACTIVATE_FAILED(work_threads_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(burden_threads_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(tcp_threads_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(proto_tcp_threads_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(http_ws_threads_))
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

    if (work_threads_ != nullptr)
    {
        if (work_threads_->GetWorkThreadGroup()->Start() != 0)
        {
            return -1;
        }
    }

    if (burden_threads_ != nullptr)
    {
        if (burden_threads_->GetBurdenThreadGroup()->Start() != 0)
        {
            return -1;
        }
    }

    if (tcp_threads_ != nullptr)
    {
        if (tcp_threads_->GetTCPThreadGroup()->Start() != 0)
        {
            return -1;
        }
    }

    if (proto_tcp_threads_ != nullptr)
    {
        if (proto_tcp_threads_->GetTCPThreadGroup()->Start() != 0)
        {
            return -1;
        }
    }

    if (http_ws_threads_ != nullptr)
    {
        if (http_ws_threads_->GetTCPThreadGroup()->Start() != 0)
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
    SAFE_FREEZE(http_ws_threads_);
    SAFE_FREEZE(proto_tcp_threads_);
    SAFE_FREEZE(tcp_threads_);
    SAFE_FREEZE(burden_threads_);
    SAFE_FREEZE(work_threads_);
    SAFE_FREEZE(global_threads_);
}

int AppFrame::NotifyStop()
{
    if (global_threads_ != nullptr && global_threads_->GetGlobalThreadGroup() != nullptr)
    {
        global_threads_->GetGlobalThreadGroup()->NotifyStop();
    }

    if (work_threads_ != nullptr && work_threads_->GetWorkThreadGroup() != nullptr)
    {
        work_threads_->GetWorkThreadGroup()->NotifyStop();
    }

    if (burden_threads_ != nullptr && burden_threads_->GetBurdenThreadGroup() != nullptr)
    {
        burden_threads_->GetBurdenThreadGroup()->NotifyStop();
    }

    if (tcp_threads_ != nullptr)
    {
        if (tcp_threads_->GetTCPThreadGroup() != nullptr)
        {
            tcp_threads_->GetTCPThreadGroup()->NotifyStop();
        }
    }

    if (proto_tcp_threads_ != nullptr)
    {
        if (proto_tcp_threads_->GetTCPThreadGroup() != nullptr)
        {
            proto_tcp_threads_->GetTCPThreadGroup()->NotifyStop();
        }
    }

    if (http_ws_threads_ != nullptr)
    {
        if (http_ws_threads_->GetTCPThreadGroup() != nullptr)
        {
            http_ws_threads_->GetTCPThreadGroup()->NotifyStop();
        }
    }

//    if (udp_threads_ != nullptr && udp_threads_->GetUdpThreadGroup() != nullptr)
//    {
//        udp_threads_->GetUdpThreadGroup()->NotifyStop();
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
        ThreadInterface* global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
        global_thread->SetReloadFinish(false);

        global_threads_->GetGlobalThreadGroup()->NotifyReload();

        // 等待global thread完成reload
        while (!global_thread->ReloadFinished())
        {
            usleep(20); // TODO
        }
    }

    if (work_threads_ != nullptr)
    {
        work_threads_->GetWorkThreadGroup()->NotifyReload();
    }

    if (burden_threads_ != nullptr)
    {
        burden_threads_->GetBurdenThreadGroup()->NotifyReload();
    }

    if (tcp_threads_ != nullptr)
    {
        tcp_threads_->GetTCPThreadGroup()->NotifyReload();
    }

    if (proto_tcp_threads_ != nullptr)
    {
        proto_tcp_threads_->GetTCPThreadGroup()->NotifyReload();
    }

    if (http_ws_threads_ != nullptr)
    {
        http_ws_threads_->GetTCPThreadGroup()->NotifyReload();
    }

//    if (udp_threads_ != nullptr)
//    {
//        udp_threads_->GetUdpThreadGroup()->NotifyReload();
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

    if (work_threads_ != nullptr && work_threads_->GetWorkThreadGroup() != nullptr)
    {
        can_exit &= (work_threads_->GetWorkThreadGroup()->CanExit() ? 1 : 0);
        LOG_DEBUG("work threads can exit: " << can_exit);
    }

    if (burden_threads_ != nullptr && burden_threads_->GetBurdenThreadGroup() != nullptr)
    {
        can_exit &= (burden_threads_->GetBurdenThreadGroup()->CanExit() ? 1 : 0);
        LOG_DEBUG("burden threads can exit: " << can_exit);
    }

    if (tcp_threads_ != nullptr)
    {
        can_exit &= (tcp_threads_->GetTCPThreadGroup()->CanExit() ? 1 : 0);
        LOG_ALWAYS("tcp threads can exit: " << can_exit);
    }

    if (proto_tcp_threads_ != nullptr)
    {
        can_exit &= (proto_tcp_threads_->GetTCPThreadGroup()->CanExit() ? 1 : 0);
        LOG_ALWAYS("proto tcp threads can exit: " << can_exit);
    }

    if (http_ws_threads_ != nullptr)
    {
        can_exit &= (http_ws_threads_->GetTCPThreadGroup()->CanExit() ? 1 : 0);
        LOG_ALWAYS("http-ws threads can exit: " << can_exit);
    }

//    if (udp_threads_ != nullptr && udp_threads_->GetUdpThreadGroup() != nullptr)
//    {
//        can_exit &= (udp_threads_->GetUdpThreadGroup()->CanExit() ? 1 : 0);
//        LOG_DEBUG("udp threads can exit: " << can_exit);
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

    if (work_threads_ != nullptr && work_threads_->GetWorkThreadGroup() != nullptr)
    {
        work_threads_->GetWorkThreadGroup()->NotifyExit();
        work_threads_->GetWorkThreadGroup()->Join();
    }

    if (burden_threads_ != nullptr && burden_threads_->GetBurdenThreadGroup() != nullptr)
    {
        burden_threads_->GetBurdenThreadGroup()->NotifyExit();
        burden_threads_->GetBurdenThreadGroup()->Join();
    }

    if (tcp_threads_ != nullptr)
    {
        tcp_threads_->GetTCPThreadGroup()->NotifyExit();
        tcp_threads_->GetTCPThreadGroup()->Join();
    }

    if (proto_tcp_threads_ != nullptr)
    {
        proto_tcp_threads_->GetTCPThreadGroup()->NotifyExit();
        proto_tcp_threads_->GetTCPThreadGroup()->Join();
    }

    if (http_ws_threads_ != nullptr)
    {
        http_ws_threads_->GetTCPThreadGroup()->NotifyExit();
        http_ws_threads_->GetTCPThreadGroup()->Join();
    }

//    if (udp_threads_ != nullptr && udp_threads_->GetUdpThreadGroup() != nullptr)
//    {
//        udp_threads_->GetUdpThreadGroup()->NotifyExit();
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

    bool tcp_exist = false;
    bool proto_tcp_exist = false;
    bool http_ws_exist = false;

//    bool udp_exist = false;

    ////////////////////////////////////////////////////////////////////////////////
    // tcp
    if (conf_mgr_.GetRawTCPAddr().length() > 0)
    {
        tcp_exist = true;

        if (0 == conf_mgr_.GetRawTCPThreadCount())
        {
            LOG_ERROR("there must be at least one tcp thread");
            return -1;
        }

        LOG_ALWAYS("tcp thread count: " << conf_mgr_.GetRawTCPThreadCount());
        app_frame_threads_count_ += conf_mgr_.GetRawTCPThreadCount();

        if (0 == conf_mgr_.GetWorkThreadCount())
        {
            // 当没有work thread时，io logic group必须存在
            if (0 == conf_mgr_.GetRawTCPLogicSoGroup().size())
            {
                LOG_ERROR("there is no work thread, so there must be at least one tcp logic so");
                return -1;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////
    // proto tcp
    if (conf_mgr_.GetProtoTCPAddr().length() > 0)
    {
        proto_tcp_exist = true;

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

    ////////////////////////////////////////////////////////////////////////////////
    // http-ws
    if (conf_mgr_.GetHTTPWSAddr().length() > 0)
    {
        http_ws_exist = true;

        if (0 == conf_mgr_.GetHTTPWSThreadCount())
        {
            LOG_ERROR("there must be at least one http-ws thread");
            return -1;
        }

        LOG_ALWAYS("http-ws thread count: " << conf_mgr_.GetHTTPWSThreadCount());
        app_frame_threads_count_ += conf_mgr_.GetHTTPWSThreadCount();

        if (0 == conf_mgr_.GetWorkThreadCount())
        {
            // 当没有work thread时，io logic group必须存在
            if (0 == conf_mgr_.GetHTTPWSLogicSoGroup().size())
            {
                LOG_ERROR("there is no work thread, so there must be at least one http-ws logic so");
                return -1;
            }
        }
    }

    // TODO

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

    ////////////////////////////////////////////////////////////////////////////////
    // work/burden thread，work/burden logic检查

    // 不允许没有work thread的情况下出现burden thread
    if (0 == conf_mgr_.GetWorkThreadCount() && conf_mgr_.GetBurdenThreadCount() > 0)
    {
        LOG_ERROR("no work thread but burden thread count is " << conf_mgr_.GetBurdenThreadCount());
        return -1;
    }

    if (conf_mgr_.GetBurdenThreadCount() > 0)
    {
        LOG_ALWAYS("burden thread count: " << conf_mgr_.GetBurdenThreadCount());
        app_frame_threads_count_ += conf_mgr_.GetBurdenThreadCount();

        // 当burden thread存在时，burden local logic是可选的，burden logic group必须存在
        if (0 == conf_mgr_.GetBurdenLogicSoGroup().size())
        {
            LOG_ERROR("there must be at least one burden logic so");
            return -1;
        }
    }

    if (conf_mgr_.GetWorkThreadCount() > 0)
    {
        LOG_ALWAYS("work thread count: " << conf_mgr_.GetWorkThreadCount());
        app_frame_threads_count_ += conf_mgr_.GetWorkThreadCount();

        if (0 == conf_mgr_.GetBurdenThreadCount() && 0 == conf_mgr_.GetRawTCPLogicSoGroup().size()
                && 0 == conf_mgr_.GetProtoTCPLogicSoGroup().size())
        {
            // 当没有burden thread和io logic group时，work logic group必须存在，work local logic是可选的
            if (0 == conf_mgr_.GetWorkLogicSoGroup().size())
            {
                LOG_ERROR("there is no burden thread and io logic group, so there must be at least one work logic so");
                return -1;
            }
        }
    }

    // tcp、http、udp可以同时存在，也可以只有一个存在，不能都不存在 TODO
    if (!tcp_exist && !proto_tcp_exist && !http_ws_exist)
    {
        LOG_ERROR("there must be one tcp or proto tcp or http-ws or udp io module");
        return -1;
    }

    return 0;
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

    work::ThreadsCtx threads_ctx;
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

int AppFrame::LoadWorkThreads()
{
    if (0 == conf_mgr_.GetWorkThreadCount())
    {
        return 0;
    }

    char WORK_THREADS_SO_PATH[MAX_PATH_LEN] = "";
    StrPrintf(WORK_THREADS_SO_PATH, sizeof(WORK_THREADS_SO_PATH), "%s/libwork_threads.so",
              app_frame_ctx_.common_component_dir);

    if (work_threads_loader_.Load(WORK_THREADS_SO_PATH) != 0)
    {
        LOG_ERROR(work_threads_loader_.GetLastErrMsg());
        return -1;
    }

    work_threads_ = (work::ThreadsInterface*) work_threads_loader_.GetModuleInterface();
    if (nullptr == work_threads_)
    {
        LOG_ERROR(work_threads_loader_.GetLastErrMsg());
        return -1;
    }

    work::ThreadsCtx threads_ctx;
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
    threads_ctx.conf.thread_count = conf_mgr_.GetWorkThreadCount();
    threads_ctx.conf.common_logic_so = conf_mgr_.GetWorkCommonLogicSo();
    threads_ctx.conf.logic_so_group = conf_mgr_.GetWorkLogicSoGroup();
    threads_ctx.logic_args = nullptr;

    if (work_threads_->Initialize(&threads_ctx) != 0)
    {
        return -1;
    }

    return 0;
}

int AppFrame::LoadBurdenThreads()
{
    if (0 == conf_mgr_.GetWorkThreadCount() || 0 == conf_mgr_.GetBurdenThreadCount())
    {
        return 0;
    }

    char BURDEN_THREADS_SO_PATH[MAX_PATH_LEN] = "";
    StrPrintf(BURDEN_THREADS_SO_PATH, sizeof(BURDEN_THREADS_SO_PATH), "%s/libburden_threads.so",
              app_frame_ctx_.common_component_dir);

    if (burden_threads_loader_.Load(BURDEN_THREADS_SO_PATH) != 0)
    {
        LOG_ERROR(burden_threads_loader_.GetLastErrMsg());
        return -1;
    }

    burden_threads_ = (burden::ThreadsInterface*) burden_threads_loader_.GetModuleInterface();
    if (nullptr == burden_threads_)
    {
        LOG_ERROR(burden_threads_loader_.GetLastErrMsg());
        return -1;
    }

    work::ThreadsCtx threads_ctx;
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

    if (burden_threads_->Initialize(&threads_ctx) != 0)
    {
        return -1;
    }

    return 0;
}

int AppFrame::LoadTCPThreads()
{
    if (0 == conf_mgr_.GetRawTCPThreadCount())
    {
        return 0;
    }

    char tcp_threads_so_path[MAX_PATH_LEN] = "";
    StrPrintf(tcp_threads_so_path, sizeof(tcp_threads_so_path), "%s/libtcp_threads.so",
              app_frame_ctx_.common_component_dir);

    if (tcp_threads_loader_.Load(tcp_threads_so_path) != 0)
    {
        LOG_ERROR(tcp_threads_loader_.GetLastErrMsg());
        return -1;
    }

    tcp_threads_ = static_cast<tcp::ThreadsInterface*>(tcp_threads_loader_.GetModuleInterface());
    if (nullptr == tcp_threads_)
    {
        LOG_ERROR(tcp_threads_loader_.GetLastErrMsg());
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
    threads_ctx.conf.io_type = IO_TYPE_TCP;
    threads_ctx.conf.use_bufferevent = conf_mgr_.RawTCPUseBufferevent();
    threads_ctx.conf.addr = conf_mgr_.GetRawTCPAddr();
    threads_ctx.conf.port = conf_mgr_.GetRawTCPPort();
    threads_ctx.conf.thread_count = conf_mgr_.GetRawTCPThreadCount();
    threads_ctx.conf.common_logic_so = conf_mgr_.GetRawTCPCommonLogicSo();
    threads_ctx.conf.logic_so_group = conf_mgr_.GetRawTCPLogicSoGroup();
    threads_ctx.logic_args = nullptr;

    if (tcp_threads_->Initialize(&threads_ctx) != 0)
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

    char proto_tcp_threads_so_path[MAX_PATH_LEN] = "";
    StrPrintf(proto_tcp_threads_so_path, sizeof(proto_tcp_threads_so_path), "%s/libproto_tcp_threads.so",
              app_frame_ctx_.common_component_dir);

    if (proto_tcp_threads_loader_.Load(proto_tcp_threads_so_path) != 0)
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

int AppFrame::LoadHTTPWSThreads()
{
    if (0 == conf_mgr_.GetHTTPWSThreadCount())
    {
        return 0;
    }

    char http_ws_threads_so_path[MAX_PATH_LEN] = "";
    StrPrintf(http_ws_threads_so_path, sizeof(http_ws_threads_so_path), "%s/libhttp_ws_threads.so",
              app_frame_ctx_.common_component_dir);

    if (http_ws_threads_loader_.Load(http_ws_threads_so_path) != 0)
    {
        LOG_ERROR(http_ws_threads_loader_.GetLastErrMsg());
        return -1;
    }

    http_ws_threads_ = static_cast<tcp::http_ws::ThreadsInterface*>(http_ws_threads_loader_.GetModuleInterface());
    if (nullptr == http_ws_threads_)
    {
        LOG_ERROR(http_ws_threads_loader_.GetLastErrMsg());
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

    if (http_ws_threads_->Initialize(&threads_ctx) != 0)
    {
        return -1;
    }

    return 0;
}

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
//    threads_ctx.cur_working_dir = app_frame_ctx_.cur_working_dir;
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

int AppFrame::CreateAllThreads()
{
    LOG_ALWAYS("AppFrame::CreateAllThreads begin");

    if (conf_mgr_.GetGlobalLogicSo().length() > 0)
    {
        if (global_threads_->CreateThreadGroup() != 0)
        {
            return -1;
        }
    }

    if (conf_mgr_.GetWorkThreadCount() > 0)
    {
        if (work_threads_->CreateThreadGroup("work thread") != 0)
        {
            return -1;
        }
    }

    if (conf_mgr_.GetBurdenThreadCount() > 0)
    {
        if (burden_threads_->CreateThreadGroup() != 0)
        {
            return -1;
        }
    }

    if (conf_mgr_.GetRawTCPThreadCount() > 0)
    {
        if (tcp_threads_->CreateThreadGroup("tcp thread") != 0)
        {
            return -1;
        }
    }

    if (conf_mgr_.GetProtoTCPThreadCount() > 0)
    {
        if (proto_tcp_threads_->CreateThreadGroup() != 0)
        {
            return -1;
        }
    }

    if (conf_mgr_.GetHTTPWSThreadCount() > 0)
    {
        if (http_ws_threads_->CreateThreadGroup() != 0)
        {
            return -1;
        }
    }

//    if (conf_mgr_.GetUdpThreadCount() > 0)
//    {
//        if (udp_threads_->CreateThreadGroup() != 0)
//        {
//            return -1;
//        }
//    }

    LOG_ALWAYS("AppFrame::CreateAllThreads end");
    return 0;
}

void AppFrame::SetThreadsRelationship()
{
    LOG_ALWAYS("AppFrame::SetThreadsRelationship begin");

    // 各线程组互相访问
    if (global_threads_ != nullptr)
    {
        work::RelatedThreadGroups related_thread_groups;

        if (work_threads_ != nullptr)
        {
            related_thread_groups.work_thread_group = work_threads_->GetWorkThreadGroup();
        }

        if (burden_threads_ != nullptr)
        {
            related_thread_groups.burden_thread_group = burden_threads_->GetBurdenThreadGroup();
        }

        if (tcp_threads_ != nullptr)
        {
            related_thread_groups.tcp_thread_group = tcp_threads_->GetTCPThreadGroup();
        }

        if (proto_tcp_threads_ != nullptr)
        {
            related_thread_groups.proto_tcp_thread_group = proto_tcp_threads_->GetTCPThreadGroup();
        }

        if (http_ws_threads_ != nullptr)
        {
            related_thread_groups.http_ws_thread_group = http_ws_threads_->GetTCPThreadGroup();
        }

        global_threads_->SetRelatedThreadGroups(&related_thread_groups);
    }

    if (work_threads_ != nullptr)
    {
        work::RelatedThreadGroups related_thread_groups;

        if (global_threads_ != nullptr)
        {
            related_thread_groups.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
            related_thread_groups.global_logic = global_threads_->GetLogic();
        }

        related_thread_groups.work_thread_group = work_threads_->GetWorkThreadGroup();

        if (burden_threads_ != nullptr)
        {
            related_thread_groups.burden_thread_group = burden_threads_->GetBurdenThreadGroup();
        }

        if (tcp_threads_ != nullptr)
        {
            related_thread_groups.tcp_thread_group = tcp_threads_->GetTCPThreadGroup();
        }

        if (proto_tcp_threads_ != nullptr)
        {
            related_thread_groups.proto_tcp_thread_group = proto_tcp_threads_->GetTCPThreadGroup();
        }

        if (http_ws_threads_ != nullptr)
        {
            related_thread_groups.http_ws_thread_group = http_ws_threads_->GetTCPThreadGroup();
        }

//        if (udp_threads_ != nullptr)
//        {
//            work_related_thread_groups.udp_thread_group = udp_threads_->GetUdpThreadGroup();
//        }

        work_threads_->SetRelatedThreadGroups(&related_thread_groups);
    }

    if (burden_threads_ != nullptr)
    {
        work::RelatedThreadGroups related_thread_groups;

        if (global_threads_ != nullptr)
        {
            related_thread_groups.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
            related_thread_groups.global_logic = global_threads_->GetLogic();
        }

        related_thread_groups.burden_thread_group = burden_threads_->GetBurdenThreadGroup();

        burden_threads_->SetRelatedThreadGroups(&related_thread_groups);
    }

    if (tcp_threads_ != nullptr)
    {
        tcp::RelatedThreadGroups related_thread_groups;

        if (global_threads_ != nullptr)
        {
            related_thread_groups.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
            related_thread_groups.global_logic = global_threads_->GetLogic();
        }

        if (work_threads_ != nullptr)
        {
            related_thread_groups.work_thread_group = work_threads_->GetWorkThreadGroup();
        }

        tcp_threads_->SetRelatedThreadGroups(&related_thread_groups);
    }

    if (proto_tcp_threads_ != nullptr)
    {
        tcp::RelatedThreadGroups related_thread_groups;

        if (global_threads_ != nullptr)
        {
            related_thread_groups.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
            related_thread_groups.global_logic = global_threads_->GetLogic();
        }

        if (work_threads_ != nullptr)
        {
            related_thread_groups.work_thread_group = work_threads_->GetWorkThreadGroup();
        }

        proto_tcp_threads_->SetRelatedThreadGroups(&related_thread_groups);
    }

    if (http_ws_threads_ != nullptr)
    {
        tcp::RelatedThreadGroups related_thread_groups;

        if (global_threads_ != nullptr)
        {
            related_thread_groups.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
            related_thread_groups.global_logic = global_threads_->GetLogic();
        }

        if (work_threads_ != nullptr)
        {
            related_thread_groups.work_thread_group = work_threads_->GetWorkThreadGroup();
        }

        http_ws_threads_->SetRelatedThreadGroups(&related_thread_groups);
    }

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

    LOG_ALWAYS("AppFrame::SetThreadsRelationship end");
}
}
