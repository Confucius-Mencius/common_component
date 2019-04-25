#ifndef BURDEN_THREADS_SRC_WORK_LOGIC_PROTO_LOGIC_ARGS_H_
#define BURDEN_THREADS_SRC_WORK_LOGIC_PROTO_LOGIC_ARGS_H_

#include "app_frame_conf_mgr_interface.h"
#include "work_threads_interface.h"

namespace work
{
struct BurdenLogicArgs
{
    app_frame::ConfMgrInterface* app_frame_conf_mgr;
    work::RelatedThreadGroups* related_thread_groups;

    BurdenLogicArgs()
    {
        app_frame_conf_mgr = nullptr;
        related_thread_groups = nullptr;
    }
};
}

#endif // BURDEN_THREADS_SRC_WORK_LOGIC_PROTO_LOGIC_ARGS_H_
