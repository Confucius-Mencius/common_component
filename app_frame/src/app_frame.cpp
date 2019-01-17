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
AppFrame::AppFrame() : app_frame_ctx_(), conf_mgr_(), tcp_threads_loader_()
{
    release_free_mem_date_ = 0;
    app_frame_threads_count_ = 0;
    tcp_threads_ = NULL;
}

AppFrame::~AppFrame()
{
}

const char* AppFrame::GetVersion() const
{
    return APP_FRAME_VERSION;
}

const char* AppFrame::GetLastErrMsg() const
{
    return NULL;
}

void AppFrame::Release()
{
    SAFE_RELEASE_MODULE(tcp_threads_, tcp_threads_loader_);

#if defined(NDEBUG)
    if (conf_mgr_.EnableCPUProfiling())
    {
        ProfilerStop();
    }

    if (conf_mgr_.EnableMemProfiling())
    {
        HeapProfilerStop();
    }
#endif

    delete this;
}

int AppFrame::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    app_frame_ctx_ = *((AppFrameCtx*) ctx);

    srand(time(NULL));

    // 初始化openssl，保证只有一次调用
    OpenSSLInitialize();

    if (pthread_mutex_init(&g_frame_threads_mutex, NULL) != 0)
    {
        const int err = errno;
        LOG_ERROR("pthread_mutex_init failed, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (pthread_cond_init(&g_frame_threads_cond, NULL) != 0)
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
        LOG_INFO("enable cpu profiling");
        ProfilerStart("cpu_profiling.prof"); // 程序启动目录下
    }

    if (conf_mgr_.EnableMemProfiling())
    {
        LOG_INFO("enable mem profiling");
        HeapProfilerStart("mem_profiling"); // prefix，程序启动目录下
    }

    LOG_INFO("before set, tcmalloc free mem release rate: " << MallocExtension::instance()->GetMemoryReleaseRate());
    MallocExtension::instance()->SetMemoryReleaseRate(5);
    LOG_INFO("after set, tcmalloc free mem release rate: " << MallocExtension::instance()->GetMemoryReleaseRate());
#endif

//    if (LoadGlobalThread() != 0)
//    {
//        return -1;
//    }

//    if (LoadWorkThreads() != 0)
//    {
//        return -1;
//    }

//    if (LoadBurdenThreads() != 0)
//    {
//        return -1;
//    }

    if (LoadTCPThreads() != 0)
    {
        return -1;
    }

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

    if (CreateThreads() != 0)
    {
        return -1;
    }

    SetThreadsRelationship();
    return 0;
}

void AppFrame::Finalize()
{
    SAFE_FINALIZE(tcp_threads_);

    conf_mgr_.Finalize();

    pthread_mutex_destroy(&g_frame_threads_mutex);
    pthread_cond_destroy(&g_frame_threads_cond);

    OpenSSLFinalize();
}

int AppFrame::Activate()
{
    // activate
    if (SAFE_ACTIVATE_FAILED(tcp_threads_))
    {
        return -1;
    }

    // ...

    // start
    tcp_threads_->GetTCPThreadGroup()->Start();
    tcp_threads_->GetListenThreadGroup()->Start();

    // 等待所有线程都启动ok
    pthread_mutex_lock(&g_frame_threads_mutex);

    while (g_frame_threads_count < app_frame_threads_count_)
    {
        pthread_cond_wait(&g_frame_threads_cond, &g_frame_threads_mutex);
    }

    pthread_mutex_unlock(&g_frame_threads_mutex);
    return 0;
}

void AppFrame::Freeze()
{
    SAFE_FREEZE(tcp_threads_);
}

int AppFrame::NotifyStop()
{
//    if (global_threads_ != NULL && global_threads_->GetGlobalThreadGroup() != NULL)
//    {
//        global_threads_->GetGlobalThreadGroup()->NotifyStop();
//    }

//    if (work_threads_ != NULL && work_threads_->GetWorkThreadGroup() != NULL)
//    {
//        work_threads_->GetWorkThreadGroup()->NotifyStop();
//    }

//    if (burden_threads_ != NULL && burden_threads_->GetBurdenThreadGroup() != NULL)
//    {
//        burden_threads_->GetBurdenThreadGroup()->NotifyStop();
//    }

    if (tcp_threads_ != NULL)
    {
        if (tcp_threads_->GetListenThreadGroup() != NULL)
        {
            tcp_threads_->GetListenThreadGroup()->NotifyStop();
        }

        if (tcp_threads_->GetTCPThreadGroup() != NULL)
        {
            tcp_threads_->GetTCPThreadGroup()->NotifyStop();
        }
    }

//    if (http_threads_ != NULL && http_threads_->GetHttpThreadGroup() != NULL)
//    {
//        http_threads_->GetHttpThreadGroup()->NotifyStop();
//    }

//    if (udp_threads_ != NULL && udp_threads_->GetUdpThreadGroup() != NULL)
//    {
//        udp_threads_->GetUdpThreadGroup()->NotifyStop();
//    }

//    if (raw_tcp_threads_ != NULL)
//    {
//        if (raw_tcp_threads_->GetListenThreadGroup() != NULL)
//        {
//            raw_tcp_threads_->GetListenThreadGroup()->NotifyStop();
//        }

//        if (raw_tcp_threads_->GetTcpThreadGroup() != NULL)
//        {
//            raw_tcp_threads_->GetTcpThreadGroup()->NotifyStop();
//        }
//    }

    return 0;
}

int AppFrame::NotifyReload(bool changed)
{
    if (changed)
    {
        conf_mgr_.Reload();
    }

#if defined(NDEBUG)
    if (conf_mgr_.ReleaseFreeMem())
    {
        const int date = GetDate();

        if (release_free_mem_date_ != date)
        {
            LOG_INFO("release free mem right now");
            MallocExtension::instance()->ReleaseFreeMemory();
            release_free_mem_date_ = date;
        }
    }
#endif

    if (!changed)
    {
        return 0;
    }

//    if (global_threads_ != NULL)
//    {
//        global_threads_->SetReloadFinish(false);

//        global_threads_->GetGlobalThreadGroup()->NotifyReload();

//        // 等待global thread完成reload
//        while (!global_threads_->ReloadFinished())
//        {
//            usleep(1000);
//        }
//    }

//    if (work_threads_ != NULL)
//    {
//        work_threads_->GetWorkThreadGroup()->NotifyReload();
//    }

//    if (burden_threads_ != NULL)
//    {
//        burden_threads_->GetBurdenThreadGroup()->NotifyReload();
//    }

    if (tcp_threads_ != NULL)
    {
        tcp_threads_->GetListenThreadGroup()->NotifyReload();
        tcp_threads_->GetTCPThreadGroup()->NotifyReload();
    }

//    if (http_threads_ != NULL)
//    {
//        http_threads_->GetHttpThreadGroup()->NotifyReload();
//    }

//    if (udp_threads_ != NULL)
//    {
//        udp_threads_->GetUdpThreadGroup()->NotifyReload();
//    }

//    if (raw_tcp_threads_ != NULL)
//    {
//        raw_tcp_threads_->GetListenThreadGroup()->NotifyReload();
//        raw_tcp_threads_->GetTcpThreadGroup()->NotifyReload();
//    }

    return 0;
}

bool AppFrame::CanExit() const
{
    int can_exit = 1;

//    if (global_threads_ != NULL && global_threads_->GetGlobalThreadGroup() != NULL)
//    {
//        can_exit &= (global_threads_->GetGlobalThreadGroup()->CanExit() ? 1 : 0);
//        LOG_DEBUG("global thread can exit: " << can_exit);
//    }

//    if (work_threads_ != NULL && work_threads_->GetWorkThreadGroup() != NULL)
//    {
//        can_exit &= (work_threads_->GetWorkThreadGroup()->CanExit() ? 1 : 0);
//        LOG_DEBUG("work threads can exit: " << can_exit);
//    }

//    if (burden_threads_ != NULL && burden_threads_->GetBurdenThreadGroup() != NULL)
//    {
//        can_exit &= (burden_threads_->GetBurdenThreadGroup()->CanExit() ? 1 : 0);
//        LOG_DEBUG("burden threads can exit: " << can_exit);
//    }

    if (tcp_threads_ != NULL)
    {
        can_exit &= (tcp_threads_->GetListenThreadGroup()->CanExit() ? 1 : 0);
        LOG_DEBUG("tcp listen thread can exit: " << can_exit);

        can_exit &= (tcp_threads_->GetTCPThreadGroup()->CanExit() ? 1 : 0);
        LOG_DEBUG("tcp threads can exit: " << can_exit);
    }

//    if (http_threads_ != NULL && http_threads_->GetHttpThreadGroup() != NULL)
//    {
//        can_exit &= (http_threads_->GetHttpThreadGroup()->CanExit() ? 1 : 0);
//        LOG_DEBUG("http threads can exit: " << can_exit);
//    }

//    if (udp_threads_ != NULL && udp_threads_->GetUdpThreadGroup() != NULL)
//    {
//        can_exit &= (udp_threads_->GetUdpThreadGroup()->CanExit() ? 1 : 0);
//        LOG_DEBUG("udp threads can exit: " << can_exit);
//    }

//    if (raw_tcp_threads_ != NULL)
//    {
//        if (raw_tcp_threads_->GetListenThreadGroup() != NULL)
//        {
//            can_exit &= (raw_tcp_threads_->GetListenThreadGroup()->CanExit() ? 1 : 0);
//            LOG_DEBUG("raw tcp listen thread can exit: " << can_exit);
//        }

//        if (raw_tcp_threads_->GetTcpThreadGroup() != NULL)
//        {
//            can_exit &= (raw_tcp_threads_->GetTcpThreadGroup()->CanExit() ? 1 : 0);
//            LOG_DEBUG("raw tcp threads can exit: " << can_exit);
//        }
//    }

    return (can_exit != 0);
}

int AppFrame::NotifyExit()
{
//    if (global_threads_ != NULL && global_threads_->GetGlobalThreadGroup() != NULL)
//    {
//        global_threads_->GetGlobalThreadGroup()->NotifyExit();
//    }

//    if (work_threads_ != NULL && work_threads_->GetWorkThreadGroup() != NULL)
//    {
//        work_threads_->GetWorkThreadGroup()->NotifyExit();
//    }

//    if (burden_threads_ != NULL && burden_threads_->GetBurdenThreadGroup() != NULL)
//    {
//        burden_threads_->GetBurdenThreadGroup()->NotifyExit();
//    }

    if (tcp_threads_ != NULL)
    {
        tcp_threads_->GetListenThreadGroup()->NotifyExit();
        tcp_threads_->GetListenThreadGroup()->Join();

        tcp_threads_->GetTCPThreadGroup()->NotifyExit();
        tcp_threads_->GetTCPThreadGroup()->Join();
    }

//    if (http_threads_ != NULL && http_threads_->GetHttpThreadGroup() != NULL)
//    {
//        http_threads_->GetHttpThreadGroup()->NotifyExit();
//    }

//    if (udp_threads_ != NULL && udp_threads_->GetUdpThreadGroup() != NULL)
//    {
//        udp_threads_->GetUdpThreadGroup()->NotifyExit();
//    }

//    if (raw_tcp_threads_ != NULL)
//    {
//        if (raw_tcp_threads_->GetListenThreadGroup() != NULL)
//        {
//            raw_tcp_threads_->GetListenThreadGroup()->NotifyExit();
//        }

//        if (raw_tcp_threads_->GetTcpThreadGroup() != NULL)
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
        LOG_INFO("global thread count: 1");
        ++app_frame_threads_count_;
    }

    bool tcp_exist = false;
//    bool http_exist = false;
//    bool udp_exist = false;
//    bool raw_tcp_exist = false;

    ////////////////////////////////////////////////////////////////////////////////
    // tcp
    if (conf_mgr_.GetTCPAddrPort().length() > 0)
    {
        tcp_exist = true; // 有tcp

        LOG_INFO("tcp listen thread count: 1");
        ++app_frame_threads_count_; // listen thread

        if (0 == conf_mgr_.GetTCPThreadCount())
        {
            LOG_ERROR("there must be at least one tcp thread");
            return -1;
        }

        LOG_INFO("tcp thread count: " << conf_mgr_.GetTCPThreadCount());
        app_frame_threads_count_ += conf_mgr_.GetTCPThreadCount();

        if (0 == conf_mgr_.GetWorkThreadCount())
        {
            // 当没有work thread时，io logic group必须存在
            if (0 == conf_mgr_.GetTCPLogicSoGroup().size())
            {
                LOG_ERROR("there is no work thread, so there must be at least one tcp logic so");
                return -1;
            }
        }
    }

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
    if (!tcp_exist)
    {
        LOG_ERROR("there must be one tcp or http/https or udp or raw tcp module");
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

//int AppFrame::LoadGlobalThread()
//{
//    if (0 == conf_mgr_.GetGlobalLogicSo().length())
//    {
//        return 0;
//    }

//    char GLOBAL_THREAD_SO_PATH[MAX_PATH_LEN] = "";
//    StrPrintf(GLOBAL_THREAD_SO_PATH, sizeof(GLOBAL_THREAD_SO_PATH), "%s/libglobal_thread.so",
//              app_frame_ctx_.common_component_dir);

//    if (global_thread_loader_.Load(GLOBAL_THREAD_SO_PATH) != 0)
//    {
//        LOG_ERROR(global_thread_loader_.GetLastErrMsg());
//        return -1;
//    }

//    global_threads_ = (global::ThreadsInterface*) global_thread_loader_.GetModuleInterface();
//    if (NULL == global_threads_)
//    {
//        LOG_ERROR(global_thread_loader_.GetLastErrMsg());
//        return -1;
//    }

//    global::ThreadsCtx threads_ctx;
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

//    if (global_threads_->Initialize(&threads_ctx) != 0)
//    {
//        return -1;
//    }

//    return 0;
//}

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
//    if (NULL == work_threads_)
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
//    if (NULL == burden_threads_)
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

int AppFrame::LoadTCPThreads()
{
    if (0 == conf_mgr_.GetTCPThreadCount())
    {
        return 0;
    }

    char TCP_THREADS_SO_PATH[MAX_PATH_LEN] = "";
    StrPrintf(TCP_THREADS_SO_PATH, sizeof(TCP_THREADS_SO_PATH), "%s/libtcp_threads.so",
              app_frame_ctx_.common_component_dir);

    if (tcp_threads_loader_.Load(TCP_THREADS_SO_PATH) != 0)
    {
        LOG_ERROR(tcp_threads_loader_.GetLastErrMsg());
        return -1;
    }

    tcp_threads_ = (tcp::ThreadsInterface*) tcp_threads_loader_.GetModuleInterface();
    if (NULL == tcp_threads_)
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
    threads_ctx.conf_mgr = &conf_mgr_;
    threads_ctx.frame_threads_count = &g_frame_threads_count;
    threads_ctx.frame_threads_mutex = &g_frame_threads_mutex;
    threads_ctx.frame_threads_cond = &g_frame_threads_cond;

    if (tcp_threads_->Initialize(&threads_ctx) != 0)
    {
        return -1;
    }

    return 0;
}

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
//    if (NULL == http_threads_)
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
//    if (NULL == udp_threads_)
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
//    if (NULL == raw_tcp_threads_)
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

int AppFrame::CreateThreads()
{
    LOG_DEBUG("AppFrame::CreateThreads begin");

//    if (conf_mgr_.GetGlobalLogicSo().length() > 0)
//    {
//        if (global_threads_->CreateThreadGroup() != 0)
//        {
//            return -1;
//        }
//    }

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

    if (conf_mgr_.GetTCPThreadCount() > 0)
    {
        if (tcp_threads_->CreateThreadGroup() != 0)
        {
            return -1;
        }
    }

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

    LOG_DEBUG("AppFrame::CreateThreads end");
    return 0;
}

void AppFrame::SetThreadsRelationship()
{
    LOG_DEBUG("AppFrame::SetThreadsRelationship begin");

    // 各线程组互相访问
//    if (global_threads_ != NULL)
//    {
//        global::RelatedThreadGroup global_related_thread_group;

//        if (burden_threads_ != NULL)
//        {
//            global_related_thread_group.burden_thread_group = burden_threads_->GetBurdenThreadGroup();
//        }

//        if (work_threads_ != NULL)
//        {
//            global_related_thread_group.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

//        if (tcp_threads_ != NULL)
//        {
//            global_related_thread_group.tcp_thread_group = tcp_threads_->GetTcpThreadGroup();
//        }

//        if (http_threads_ != NULL)
//        {
//            global_related_thread_group.http_thread_group = http_threads_->GetHttpThreadGroup();
//        }

//        if (udp_threads_ != NULL)
//        {
//            global_related_thread_group.udp_thread_group = udp_threads_->GetUdpThreadGroup();
//        }

//        if (raw_tcp_threads_ != NULL)
//        {
//            global_related_thread_group.tcp_thread_group = raw_tcp_threads_->GetTcpThreadGroup();
//        }

//        global_threads_->SetRelatedThreadGroup(&global_related_thread_group);
//    }

//    if (work_threads_ != NULL)
//    {
//        work::RelatedThreadGroup work_related_thread_group;

//        if (global_threads_ != NULL)
//        {
//            work_related_thread_group.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            work_related_thread_group.global_logic = global_threads_->GetLogic();
//        }

//        if (burden_threads_ != NULL)
//        {
//            work_related_thread_group.burden_thread_group = burden_threads_->GetBurdenThreadGroup();
//        }

//        if (tcp_threads_ != NULL)
//        {
//            work_related_thread_group.tcp_thread_group = tcp_threads_->GetTcpThreadGroup();
//        }

//        if (http_threads_ != NULL)
//        {
//            work_related_thread_group.http_thread_group = http_threads_->GetHttpThreadGroup();
//        }

//        if (udp_threads_ != NULL)
//        {
//            work_related_thread_group.udp_thread_group = udp_threads_->GetUdpThreadGroup();
//        }

//        work_threads_->SetRelatedThreadGroup(&work_related_thread_group);
//    }

//    if (burden_threads_ != NULL)
//    {
//        burden::RelatedThreadGroup burden_related_thread_group;

//        if (global_threads_ != NULL)
//        {
//            burden_related_thread_group.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            burden_related_thread_group.global_logic = global_threads_->GetLogic();
//        }

//        if (work_threads_ != NULL)
//        {
//            burden_related_thread_group.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

//        if (tcp_threads_ != NULL)
//        {
//            burden_related_thread_group.tcp_thread_group = tcp_threads_->GetTcpThreadGroup();
//        }

//        if (http_threads_ != NULL)
//        {
//            burden_related_thread_group.http_thread_group = http_threads_->GetHttpThreadGroup();
//        }

//        if (udp_threads_ != NULL)
//        {
//            burden_related_thread_group.udp_thread_group = udp_threads_->GetUdpThreadGroup();
//        }

//        burden_threads_->SetRelatedThreadGroup(&burden_related_thread_group);
//    }

    if (tcp_threads_ != NULL)
    {
        tcp::RelatedThreadGroups tcp_related_thread_groups;

//        if (global_threads_ != NULL)
//        {
//            tcp_related_thread_groups.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            tcp_related_thread_groups.global_logic = global_threads_->GetLogic();
//        }

//        if (work_threads_ != NULL)
//        {
//            tcp_related_thread_groups.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

        tcp_threads_->SetRelatedThreadGroups(&tcp_related_thread_groups);
    }

//    if (http_threads_ != NULL)
//    {
//        http::RelatedThreadGroup http_related_thread_group;

//        if (global_threads_ != NULL)
//        {
//            http_related_thread_group.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            http_related_thread_group.global_logic = global_threads_->GetLogic();
//        }

//        if (work_threads_ != NULL)
//        {
//            http_related_thread_group.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

//        http_threads_->SetRelatedThreadGroup(&http_related_thread_group);
//    }

//    if (udp_threads_ != NULL)
//    {
//        udp::RelatedThreadGroup udp_related_thread_group;

//        if (global_threads_ != NULL)
//        {
//            udp_related_thread_group.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            udp_related_thread_group.global_logic = global_threads_->GetLogic();
//        }

//        if (work_threads_ != NULL)
//        {
//            udp_related_thread_group.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

//        udp_threads_->SetRelatedThreadGroup(&udp_related_thread_group);
//    }

//    if (raw_tcp_threads_ != NULL)
//    {
//        tcp::RelatedThreadGroup tcp_related_thread_group;

//        if (global_threads_ != NULL)
//        {
//            tcp_related_thread_group.global_thread = global_threads_->GetGlobalThreadGroup()->GetThread(0);
//            tcp_related_thread_group.global_logic = global_threads_->GetLogic();
//        }

//        if (work_threads_ != NULL)
//        {
//            tcp_related_thread_group.work_thread_group = work_threads_->GetWorkThreadGroup();
//        }

//        raw_tcp_threads_->SetRelatedThreadGroup(&tcp_related_thread_group);
//    }

    LOG_DEBUG("AppFrame::SetThreadsRelationship end");
}
}
