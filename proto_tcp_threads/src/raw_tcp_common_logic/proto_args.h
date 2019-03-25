#ifndef PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_PROTO_ARGS_H_
#define PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_PROTO_ARGS_H_

#include "app_frame_conf_mgr_interface.h"
#include "proto_msg_codec_interface.h"
#include "proto_tcp_scheduler_interface.h"

namespace tcp
{
namespace raw
{
struct ProtoArgs
{
    app_frame::ConfMgrInterface* app_frame_conf_mgr;
    ::proto::MsgCodecInterface* proto_msg_codec;
    tcp::proto::SchedulerInterface* scheduler;

    ProtoArgs()
    {
        app_frame_conf_mgr = NULL;
        proto_msg_codec = NULL;
        scheduler = NULL;
    }
};
}
}

#endif // PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_PROTO_ARGS_H_
