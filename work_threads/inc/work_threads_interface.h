#ifndef WORK_THREADS_INC_WORK_THREADS_INTERFACE_H_
#define WORK_THREADS_INC_WORK_THREADS_INTERFACE_H_

#include "thread_center_interface.h"

class ConfCenterInterface;

namespace global
{
class LogicInterface;
}

namespace app_frame
{
class ConfMgrInterface;
}

namespace work
{
struct ThreadsCtx
{
    int argc;
    char** argv;
    const char* common_component_dir;
    const char* cur_working_dir;
    const char* app_name;
    ConfCenterInterface* conf_center;
    ThreadCenterInterface* thread_center;
    app_frame::ConfMgrInterface* app_frame_conf_mgr;
    int* app_frame_threads_count;
    pthread_mutex_t* app_frame_threads_sync_mutex;
    pthread_cond_t* app_frame_threads_sync_cond;

    ThreadsCtx()
    {
        argc = 0;
        argv = nullptr;
        common_component_dir = nullptr;
        cur_working_dir = nullptr;
        app_name = nullptr;
        conf_center = nullptr;
        thread_center = nullptr;
        app_frame_conf_mgr = nullptr;
        app_frame_threads_count = nullptr;
        app_frame_threads_sync_mutex = nullptr;
        app_frame_threads_sync_cond = nullptr;
    }
};

struct RelatedThreadGroups
{
    ThreadInterface* global_thread;
    global::LogicInterface* global_logic;
    ThreadGroupInterface* burden_thread_group;
    ThreadGroupInterface* raw_tcp_thread_group;
    ThreadGroupInterface* proto_tcp_thread_group;

    RelatedThreadGroups()
    {
        global_thread = nullptr;
        global_logic = nullptr;
        burden_thread_group = nullptr;
        raw_tcp_thread_group = nullptr;
        proto_tcp_thread_group = nullptr;
    }
};

class ThreadsInterface : public ModuleInterface
{
public:
    virtual ~ThreadsInterface()
    {
    }

    virtual int CreateThreadGroup() = 0;
    virtual void SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_groups) = 0;
    virtual ThreadGroupInterface* GetWorkThreadGroup() const = 0;
};
}

#endif // WORK_THREADS_INC_WORK_THREADS_INTERFACE_H_
