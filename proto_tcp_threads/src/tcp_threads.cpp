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
Threads::Threads() : threads_ctx_(), related_thread_groups_(), proto_msg_codec_loader_(),
    raw_tcp_threads_loader_(), proto_tcp_logic_args_()
{
    proto_msg_codec_ = NULL;
    raw_tcp_threads_ = NULL;
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
    return NULL;
}

void Threads::Release()
{
    SAFE_RELEASE(raw_tcp_threads_);
    SAFE_RELEASE(proto_msg_codec_);
    delete this;
}

int Threads::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    threads_ctx_ = *(static_cast<const ThreadsCtx*>(ctx));

    if (LoadProtoMsgCodec() != 0)
    {
        return -1;
    }

    if (LoadRawTCPThreads() != 0)
    {
        return -1;
    }

    return 0;
}

void Threads::Finalize()
{
    SAFE_FINALIZE(raw_tcp_threads_);
    SAFE_FINALIZE(proto_msg_codec_);
}

int Threads::Activate()
{
    if (SAFE_ACTIVATE_FAILED(proto_msg_codec_) != 0)
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(raw_tcp_threads_) != 0)
    {
        return -1;
    }

    return 0;
}

void Threads::Freeze()
{
    SAFE_FREEZE(raw_tcp_threads_);
    SAFE_FREEZE(proto_msg_codec_);
}

int Threads::CreateThreadGroup(const char* name_prefix)
{
    if (raw_tcp_threads_->CreateThreadGroup(name_prefix) != 0)
    {
        return -1;
    }

    return 0;
}

void Threads::SetRelatedThreadGroups(const tcp::RelatedThreadGroups* related_thread_groups)
{
    if (NULL == related_thread_groups)
    {
        return;
    }

    raw_tcp_threads_->SetRelatedThreadGroups(related_thread_groups);
    related_thread_groups_ = *related_thread_groups;
}

ThreadGroupInterface* Threads::GetListenThreadGroup() const
{
    return raw_tcp_threads_->GetListenThreadGroup();
}

ThreadGroupInterface* Threads::GetTCPThreadGroup() const
{
    return raw_tcp_threads_->GetTCPThreadGroup();
}

int Threads::LoadProtoMsgCodec()
{
    char PROTO_MSG_CODEC_SO_PATH[MAX_PATH_LEN] = "";
    StrPrintf(PROTO_MSG_CODEC_SO_PATH, sizeof(PROTO_MSG_CODEC_SO_PATH), "%s/libproto_msg_codec.so",
              threads_ctx_.common_component_dir);

    if (proto_msg_codec_loader_.Load(PROTO_MSG_CODEC_SO_PATH) != 0)
    {
        LOG_ERROR(proto_msg_codec_loader_.GetLastErrMsg());
        return -1;
    }

    proto_msg_codec_ = static_cast<::proto::MsgCodecInterface*>(proto_msg_codec_loader_.GetModuleInterface());
    if (NULL == proto_msg_codec_)
    {
        LOG_ERROR(proto_msg_codec_loader_.GetLastErrMsg());
        return -1;
    }

    ::proto::MsgCodecCtx proto_msg_codec_ctx;
    proto_msg_codec_ctx.max_msg_body_len = threads_ctx_.app_frame_conf_mgr->GetProtoMaxMsgBodyLen();
    proto_msg_codec_ctx.do_checksum = threads_ctx_.app_frame_conf_mgr->ProtoDoChecksum();

    if (proto_msg_codec_->Initialize(&proto_msg_codec_ctx) != 0)
    {
        return -1;
    }

    return 0;
}

int Threads::LoadRawTCPThreads()
{
    char RAW_TCP_THREADS_SO_PATH[MAX_PATH_LEN] = "";
    StrPrintf(RAW_TCP_THREADS_SO_PATH, sizeof(RAW_TCP_THREADS_SO_PATH), "%s/libraw_tcp_threads.so",
              threads_ctx_.common_component_dir);

    if (raw_tcp_threads_loader_.Load(RAW_TCP_THREADS_SO_PATH) != 0)
    {
        LOG_ERROR(raw_tcp_threads_loader_.GetLastErrMsg());
        return -1;
    }

    raw_tcp_threads_ = static_cast<tcp::raw::ThreadsInterface*>(raw_tcp_threads_loader_.GetModuleInterface());
    if (NULL == raw_tcp_threads_)
    {
        LOG_ERROR(raw_tcp_threads_loader_.GetLastErrMsg());
        return -1;
    }

    tcp::ThreadsCtx raw_threads_ctx = threads_ctx_;
    raw_threads_ctx.conf.io_type = IO_TYPE_PROTO_TCP;
    raw_threads_ctx.conf.addr = threads_ctx_.app_frame_conf_mgr->GetProtoTCPAddr();
    raw_threads_ctx.conf.port = threads_ctx_.app_frame_conf_mgr->GetProtoTCPPort();
    raw_threads_ctx.conf.thread_count = threads_ctx_.app_frame_conf_mgr->GetProtoTCPThreadCount();
    raw_threads_ctx.conf.common_logic_so = std::string(threads_ctx_.common_component_dir) + "/libproto_raw_tcp_common_logic.so";

    proto_tcp_logic_args_.app_frame_conf_mgr = threads_ctx_.app_frame_conf_mgr;
    proto_tcp_logic_args_.proto_msg_codec = proto_msg_codec_;
    proto_tcp_logic_args_.related_thread_groups = &related_thread_groups_;

    raw_threads_ctx.logic_args = &proto_tcp_logic_args_;

    if (raw_tcp_threads_->Initialize(&raw_threads_ctx) != 0)
    {
        return -1;
    }

    return 0;
}
}
}
