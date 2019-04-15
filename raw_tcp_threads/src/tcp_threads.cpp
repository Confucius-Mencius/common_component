#include "tcp_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "thread_sink.h"
#include "version.h"

namespace tcp
{
namespace raw
{
Threads::Threads() : threads_ctx_(), related_thread_groups_()
{
//    listen_sock_fd_ = -1;
    tcp_thread_group_ = nullptr;
}

Threads::~Threads()
{
}

const char* Threads::GetVersion() const
{
    return RAW_TCP_THREADS_RAW_TCP_THREADS_VERSION;
}

const char* Threads::GetLastErrMsg() const
{
    return nullptr;
}

void Threads::Release()
{
    SAFE_RELEASE(tcp_thread_group_);
    delete this;
}

int Threads::Initialize(const void* ctx)
{
    if (nullptr == ctx)
    {
        return -1;
    }

    threads_ctx_ = *(static_cast<const ThreadsCtx*>(ctx));

//    const std::string tcp_addr_port = threads_ctx_.conf.addr + ":" + std::to_string(threads_ctx_.conf.port);
//    LOG_ALWAYS("tcp listen addr port: " << tcp_addr_port);

//    listen_sock_fd_ = socket(AF_INET, SOCK_STREAM, 0);
//    if (listen_sock_fd_ < 0)
//    {
//        const int err = EVUTIL_SOCKET_ERROR();
//        LOG_ERROR("failed to create tcp listen socket, errno: " << err
//                  << ", err msg: " << evutil_socket_error_to_string(err));
//        return -1;
//    }

//    int ret = -1;

//    do
//    {
//        if (evutil_make_socket_nonblocking(listen_sock_fd_) != 0)
//        {
//            const int err = EVUTIL_SOCKET_ERROR();
//            LOG_ERROR("failed to make tcp listen socket non blocking, errno: " << err
//                      << ", err msg: " << evutil_socket_error_to_string(err));
//            break;
//        }

//        // TODO 测试 ip:port, 域名:port
//        struct sockaddr_in listen_sock_addr;
//        int listen_sock_addr_len = sizeof(listen_sock_addr);

//        if (evutil_parse_sockaddr_port(tcp_addr_port.c_str(),
//                                       (struct sockaddr*) &listen_sock_addr, &listen_sock_addr_len) != 0)
//        {
//            const int err = EVUTIL_SOCKET_ERROR();
//            LOG_ERROR("failed to parse tcp listen socket addr port: " << tcp_addr_port
//                      << ", errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
//            break;
//        }

//        if (bind(listen_sock_fd_, (struct sockaddr*) &listen_sock_addr, listen_sock_addr_len) != 0)
//        {
//            const int err = errno;
//            LOG_ERROR("failed to bind tcp listen socket addr port: " << tcp_addr_port << ", errno: " << err
//                      << ", err msg: " << evutil_socket_error_to_string(err));
//            break;
//        }

//        ret = 0;
//    } while (0);

//    if (ret != 0)
//    {
//        evutil_closesocket(listen_sock_fd_);
//        listen_sock_fd_ = -1;
//    }

    return 0;
}

void Threads::Finalize()
{
    SAFE_FINALIZE(tcp_thread_group_);

//    if (listen_sock_fd_ != -1)
//    {
//        evutil_closesocket(listen_sock_fd_);
//        listen_sock_fd_ = -1;
//    }
}

int Threads::Activate()
{
    if (SAFE_ACTIVATE_FAILED(tcp_thread_group_) != 0)
    {
        return -1;
    }

    return 0;
}

void Threads::Freeze()
{
    SAFE_FREEZE(tcp_thread_group_);
}

int Threads::CreateThreadGroup(const char* name_prefix)
{
    int ret = -1;

    do
    {
        ThreadGroupCtx thread_group_ctx;
        thread_group_ctx.common_component_dir = threads_ctx_.common_component_dir;
        thread_group_ctx.enable_cpu_profiling = threads_ctx_.app_frame_conf_mgr->EnableCPUProfiling();
        thread_group_ctx.thread_name = std::string(name_prefix) + " thread";
        thread_group_ctx.thread_count = threads_ctx_.conf.thread_count;
        thread_group_ctx.thread_sink_creator = ThreadSink::Create;
        thread_group_ctx.threads_ctx = &threads_ctx_;

        tcp_thread_group_ = threads_ctx_.thread_center->CreateThreadGroup(&thread_group_ctx);
        if (nullptr == tcp_thread_group_)
        {
            break;
        }

        for (int i = 0; i < tcp_thread_group_->GetThreadCount(); ++i)
        {
            ThreadSink* thread_sink = static_cast<ThreadSink*>(tcp_thread_group_->GetThread(i)->GetThreadSink());
//            thread_sink->SetListenSocketFD(listen_sock_fd_);
            thread_sink->SetTCPThreadGroup(tcp_thread_group_);
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (tcp_thread_group_ != nullptr)
        {
            SAFE_DESTROY(tcp_thread_group_);
        }
    }

    return ret;
}

void Threads::SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_groups)
{
    if (nullptr == related_thread_groups)
    {
        return;
    }

    related_thread_groups_ = *related_thread_groups;

    for (int i = 0; i < tcp_thread_group_->GetThreadCount(); ++i)
    {
        ThreadSink* thread_sink = static_cast<ThreadSink*>(tcp_thread_group_->GetThread(i)->GetThreadSink());
        thread_sink->SetRelatedThreadGroups(&related_thread_groups_);
    }
}

ThreadGroupInterface* Threads::GetTCPThreadGroup() const
{
    return tcp_thread_group_;
}
}
}
