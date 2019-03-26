#ifndef PROTO_TCP_THREADS_SRC_TCP_THREADS_H_
#define PROTO_TCP_THREADS_SRC_TCP_THREADS_H_

#include "module_loader.h"
#include "proto_msg_codec_interface.h"
#include "proto_tcp_threads_interface.h"
#include "raw_tcp_common_logic/proto_args.h"

namespace tcp
{
namespace proto
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
    int CreateThreadGroup(const char* name_prefix) override;
    void SetRelatedThreadGroups(const tcp::RelatedThreadGroups* related_thread_groups) override;
    ThreadGroupInterface* GetListenThreadGroup() const override;
    ThreadGroupInterface* GetTCPThreadGroup() const override;

private:
    int LoadProtoMsgCodec();
    int LoadRawTCPThreads();

private:
    ThreadsCtx threads_ctx_;
    tcp::RelatedThreadGroups related_thread_groups_;

    ModuleLoader proto_msg_codec_loader_;
    ::proto::MsgCodecInterface* proto_msg_codec_;

    ModuleLoader raw_tcp_threads_loader_;
    tcp::raw::ThreadsInterface* raw_tcp_threads_;

    tcp::raw::ProtoArgs proto_tcp_logic_args_;
};
}
}

#endif // PROTO_TCP_THREADS_SRC_TCP_THREADS_H_