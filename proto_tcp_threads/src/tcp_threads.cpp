#include "tcp_threads.h"
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "log_util.h"
#include "mem_util.h"
#include "str_util.h"
#include "version.h"

namespace tcp
{
namespace proto
{
Threads::Threads() : threads_ctx_(), related_thread_groups_(), tcp_threads_loader_(), proto_logic_args_()
{
    tcp_threads_ = nullptr;
}

Threads::~Threads()
{
}

const char* Threads::GetVersion() const
{
    return PROTO_TCP_THREADS_PROTO_TCP_THREADS_VERSION;
}

const char* Threads::GetLastErrMsg() const
{
    return nullptr;
}

void Threads::Release()
{
    SAFE_RELEASE(tcp_threads_);
    delete this;
}

int Threads::Initialize(const void* ctx)
{
    if (nullptr == ctx)
    {
        return -1;
    }

    threads_ctx_ = *(static_cast<const ThreadsCtx*>(ctx));

    if (LoadTCPThreads() != 0)
    {
        return -1;
    }

    return 0;
}

void Threads::Finalize()
{
    SAFE_FINALIZE(tcp_threads_);
}

int Threads::Activate()
{
    if (SAFE_ACTIVATE_FAILED(tcp_threads_))
    {
        return -1;
    }

    return 0;
}

void Threads::Freeze()
{
    SAFE_FREEZE(tcp_threads_);
}

int Threads::CreateThreadGroup()
{
    if (tcp_threads_->CreateThreadGroup("proto tcp thread") != 0)
    {
        return -1;
    }

    return 0;
}

void Threads::SetRelatedThreadGroups(const tcp::RelatedThreadGroups* related_thread_groups)
{
    if (nullptr == related_thread_groups)
    {
        return;
    }

    tcp_threads_->SetRelatedThreadGroups(related_thread_groups);
    related_thread_groups_ = *related_thread_groups;
}

ThreadGroupInterface* Threads::GetTCPThreadGroup() const
{
    return tcp_threads_->GetTCPThreadGroup();
}

int Threads::LoadTCPThreads()
{
    char tcp_threads_so_path[MAX_PATH_LEN] = "";
    StrPrintf(tcp_threads_so_path, sizeof(tcp_threads_so_path), "%s/libtcp_threads.so",
              threads_ctx_.common_component_dir);

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

    tcp::ThreadsCtx tcp_threads_ctx = threads_ctx_;
    tcp_threads_ctx.conf.io_type = IO_TYPE_PROTO_TCP;
    tcp_threads_ctx.conf.use_bufferevent = threads_ctx_.app_frame_conf_mgr->ProtoTCPUseBufferevent();
    tcp_threads_ctx.conf.addr = threads_ctx_.app_frame_conf_mgr->GetProtoTCPAddr();
    tcp_threads_ctx.conf.port = threads_ctx_.app_frame_conf_mgr->GetProtoTCPPort();
    tcp_threads_ctx.conf.thread_count = threads_ctx_.app_frame_conf_mgr->GetProtoTCPThreadCount();
    tcp_threads_ctx.conf.common_logic_so = std::string(threads_ctx_.common_component_dir) + "/libproto_tcp_common_logic.so";

    proto_logic_args_.app_frame_conf_mgr = threads_ctx_.app_frame_conf_mgr;
    proto_logic_args_.related_thread_groups = &related_thread_groups_;

    tcp_threads_ctx.logic_args = &proto_logic_args_;

    if (tcp_threads_->Initialize(&tcp_threads_ctx) != 0)
    {
        return -1;
    }

    return 0;
}
}
}
