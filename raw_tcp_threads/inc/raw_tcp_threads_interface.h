#ifndef RAW_TCP_THREADS_INC_RAW_TCP_THREADS_INTERFACE_H_
#define RAW_TCP_THREADS_INC_RAW_TCP_THREADS_INTERFACE_H_

#include "thread_center_interface.h"
#include "vector_types.h"

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
struct Conf
{
    std::string addr;
    unsigned int port;
    int thread_count;
    std::string common_logic_so;
    StrGroup logic_so_group;

    Conf() : addr(), common_logic_so(), logic_so_group()
    {
        port = 0;
        thread_count = 0;
    }
};

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
    Conf conf;
    const void* logic_args;

    ThreadsCtx() : conf()
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
        logic_args = NULL;
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

namespace raw
{
class ThreadsInterface : public ModuleInterface
{
public:
    virtual ~ThreadsInterface()
    {
    }

    virtual int CreateThreadGroup(const char* name_prefix) = 0;
    virtual void SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_groups) = 0;
    virtual ThreadGroupInterface* GetListenThreadGroup() const = 0;
    virtual ThreadGroupInterface* GetTCPThreadGroup() const = 0;
};
}
}

#endif // RAW_TCP_THREADS_INC_RAW_TCP_THREADS_INTERFACE_H_
