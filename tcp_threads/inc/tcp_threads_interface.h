#ifndef TCP_THREADS_INC_TCP_THREADS_INTERFACE_H_
#define TCP_THREADS_INC_TCP_THREADS_INTERFACE_H_

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

namespace tcp
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
    int* frame_threads_count;
    pthread_mutex_t* frame_threads_mutex;
    pthread_cond_t* frame_threads_cond;

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
        frame_threads_count = NULL;
        frame_threads_mutex = NULL;
        frame_threads_cond = NULL;
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
    virtual ThreadGroupInterface* GetListenThreadGroup() const = 0;
    virtual ThreadGroupInterface* GetTCPThreadGroup() const = 0;
};
}

#endif // TCP_THREADS_INC_TCP_THREADS_INTERFACE_H_
