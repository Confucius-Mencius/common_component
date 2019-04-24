#ifndef GLOBAL_THREADS_SRC_WORK_COMMON_LOGIC_PROTO_LOGIC_ARGS_H_
#define GLOBAL_THREADS_SRC_WORK_COMMON_LOGIC_PROTO_LOGIC_ARGS_H_

#include "app_frame_conf_mgr_interface.h"
#include "work_threads_interface.h"

namespace global
{
class LogicInterface;
}

namespace work
{
struct GlobalLogicArgs
{
    app_frame::ConfMgrInterface* app_frame_conf_mgr;
    work::RelatedThreadGroups* related_thread_groups;
    global::LogicInterface* global_logic;

    GlobalLogicArgs()
    {
        app_frame_conf_mgr = nullptr;
        related_thread_groups = nullptr;
        global_logic = nullptr;
    }
};
}

#endif // GLOBAL_THREADS_SRC_WORK_COMMON_LOGIC_PROTO_LOGIC_ARGS_H_
