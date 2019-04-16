#ifndef UDP_THREADS_INC_UDP_THREADS_INTERFACE_H_
#define UDP_THREADS_INC_UDP_THREADS_INTERFACE_H_

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

namespace udp
{
struct Conf
{
    IOType io_type;
    std::string addr;
    unsigned int port;
    int thread_count;
    std::string common_logic_so;
    StrGroup logic_so_group;

    Conf() : addr(), common_logic_so(), logic_so_group()
    {
        io_type = IO_TYPE_MIN;
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

    // 下面两个字段是为了支持多种类型的udp io
    Conf conf;
    const void* logic_args;

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
        logic_args = nullptr;
    }
};

struct RelatedThreadGroups
{
    ThreadInterface* global_thread;
    global::LogicInterface* global_logic;
    ThreadGroupInterface* work_thread_group;

    RelatedThreadGroups()
    {
        global_thread = nullptr;
        global_logic = nullptr;
        work_thread_group = nullptr;
    }
};

class ThreadsInterface : public ModuleInterface
{
public:
    virtual ~ThreadsInterface()
    {
    }

    virtual int CreateThreadGroup(const char* name_prefix) = 0;
    virtual void SetRelatedThreadGroups(const RelatedThreadGroups* related_thread_groups) = 0;
    virtual ThreadGroupInterface* GetUDPThreadGroup() const = 0;
};
}

#endif // UDP_THREADS_INC_UDP_THREADS_INTERFACE_H_
