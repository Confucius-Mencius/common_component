#ifndef PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_PROTO_ARGS_H_
#define PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_PROTO_ARGS_H_

#include "app_frame_conf_mgr_interface.h"
#include "proto_msg_codec.h"
#include "raw_tcp_threads_interface.h"

namespace tcp
{
namespace raw
{
struct ProtoArgs
{
    app_frame::ConfMgrInterface* app_frame_conf_mgr;
    tcp::RelatedThreadGroups* related_thread_groups;

    ProtoArgs()
    {
        app_frame_conf_mgr = nullptr;
        related_thread_groups = nullptr;
    }
};
}
}

#endif // PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_PROTO_ARGS_H_
