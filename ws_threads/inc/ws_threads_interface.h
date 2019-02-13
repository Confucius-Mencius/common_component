#ifndef WS_THREADS_INC_WS_THREADS_INTERFACE_H_
#define WS_THREADS_INC_WS_THREADS_INTERFACE_H_

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

namespace ws
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
    app_frame::ConfMgrInterface* conf_mgr;
    int* app_frame_threads_count;
    pthread_mutex_t* app_frame_threads_sync_mutex;
    pthread_cond_t* app_frame_threads_sync_cond;

    ThreadsCtx()
    {
        argc = 0;
        argv = NULL;
        common_component_dir = NULL;
        cur_working_dir = NULL;
        app_name = NULL;
        conf_center = NULL;
        thread_center = NULL;
        conf_mgr = NULL;
        app_frame_threads_count = NULL;
        app_frame_threads_sync_mutex = NULL;
        app_frame_threads_sync_cond = NULL;
    }
};

struct RelatedThreadGroups
{
    ThreadInterface* global_thread;
    global::LogicInterface* global_logic;
    ThreadGroupInterface* work_threads;

    RelatedThreadGroups()
    {
        global_thread = NULL;
        global_logic = NULL;
        work_threads = NULL;
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
    virtual ThreadGroupInterface* GetWSThreadGroup() const = 0;
};
}

#endif // WS_THREADS_INC_WS_THREADS_INTERFACE_H_
