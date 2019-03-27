#ifndef GLOBAL_THREAD_INC_GLOBAL_THREADS_INTERFACE_H_
#define GLOBAL_THREAD_INC_GLOBAL_THREADS_INTERFACE_H_

#include "thread_center_interface.h"

class ConfCenterInterface;

namespace app_frame
{
class ConfMgrInterface;
}

namespace global
{
class LogicInterface;

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
        argv = NULL;
        common_component_dir = NULL;
        cur_working_dir = NULL;
        app_name = NULL;
        conf_center = NULL;
        thread_center = NULL;
        app_frame_conf_mgr = NULL;
        app_frame_threads_count = NULL;
        app_frame_threads_sync_mutex = NULL;
        app_frame_threads_sync_cond = NULL;
    }
};

struct RelatedThreadGroups
{
    ThreadGroupInterface* work_threads;
    ThreadGroupInterface* burden_threads;
    ThreadGroupInterface* proto_tcp_threads;

    RelatedThreadGroups()
    {
        work_threads = NULL;
        burden_threads = NULL;
        proto_tcp_threads = NULL;
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

    virtual ThreadGroupInterface* GetGlobalThreadGroup() const = 0;
    virtual LogicInterface* GetLogic() const = 0; // global logic

    virtual void SetReloadFinish(bool finished) = 0;
    virtual bool ReloadFinished() = 0;
};
}

#endif // GLOBAL_THREAD_INC_GLOBAL_THREADS_INTERFACE_H_
