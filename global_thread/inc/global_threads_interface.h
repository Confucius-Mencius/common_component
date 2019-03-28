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
    ThreadGroupInterface* work_thread_group;
    ThreadGroupInterface* burden_thread_group;
    ThreadGroupInterface* proto_tcp_thread_group;

    RelatedThreadGroups()
    {
        work_thread_group = nullptr;
        burden_thread_group = nullptr;
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

    virtual ThreadGroupInterface* GetGlobalThreadGroup() const = 0;
    virtual LogicInterface* GetLogic() const = 0; // global logic

    virtual void SetReloadFinish(bool finished) = 0;
    virtual bool ReloadFinished() = 0;
};
}

#endif // GLOBAL_THREAD_INC_GLOBAL_THREADS_INTERFACE_H_
