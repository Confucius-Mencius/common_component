#ifndef HTTP_WS_THREADS_SRC_TCP_COMMON_LOGIC_HTTP_WS_LOGIC_ARGS_H_
#define HTTP_WS_THREADS_SRC_TCP_COMMON_LOGIC_HTTP_WS_LOGIC_ARGS_H_

#include "app_frame_conf_mgr_interface.h"
#include "tcp_threads_interface.h"

namespace tcp
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

#endif // HTTP_WS_THREADS_SRC_TCP_COMMON_LOGIC_HTTP_WS_LOGIC_ARGS_H_
