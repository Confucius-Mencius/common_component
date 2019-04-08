#ifndef HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_HTTP_WS_LOGIC_ARGS_H_
#define HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_HTTP_WS_LOGIC_ARGS_H_

#include "app_frame_conf_mgr_interface.h"
#include "raw_tcp_threads_interface.h"

namespace tcp
{
namespace raw
{
struct HTTPWSLogicArgs
{
    app_frame::ConfMgrInterface* app_frame_conf_mgr;
    tcp::RelatedThreadGroups* related_thread_groups;

    HTTPWSLogicArgs()
    {
        app_frame_conf_mgr = nullptr;
        related_thread_groups = nullptr;
    }
};
}
}

#endif // HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_HTTP_WS_LOGIC_ARGS_H_
