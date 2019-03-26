#ifndef PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_PROTO_ARGS_H_
#define PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_PROTO_ARGS_H_

#include "app_frame_conf_mgr_interface.h"
#include "proto_msg_codec_interface.h"
#include "raw_tcp_threads_interface.h"

namespace tcp
{
namespace raw
{
struct ProtoArgs
{
    app_frame::ConfMgrInterface* app_frame_conf_mgr;
    ::proto::MsgCodecInterface* proto_msg_codec;
    tcp::RelatedThreadGroups* related_thread_groups;

    ProtoArgs()
    {
        app_frame_conf_mgr = NULL;
        proto_msg_codec = NULL;
        related_thread_groups = NULL;
    }
};
}
}

#endif // PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_PROTO_ARGS_H_