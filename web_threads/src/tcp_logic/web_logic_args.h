#ifndef WEB_THREADS_SRC_TCP_LOGIC_WEB_LOGIC_ARGS_H_
#define WEB_THREADS_SRC_TCP_LOGIC_WEB_LOGIC_ARGS_H_

#include "app_frame_conf_mgr_interface.h"
#include "tcp_threads_interface.h"

namespace tcp
{
struct WebLogicArgs
{
    app_frame::ConfMgrInterface* app_frame_conf_mgr;
    tcp::RelatedThreadGroups* related_thread_groups;

    WebLogicArgs()
    {
        app_frame_conf_mgr = nullptr;
        related_thread_groups = nullptr;
    }
};
}

#endif // WEB_THREADS_SRC_TCP_LOGIC_WEB_LOGIC_ARGS_H_
