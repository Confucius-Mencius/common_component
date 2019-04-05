#ifndef UDP_THREADS_INC_UDP_THREADS_INTERFACE_H_
#define UDP_THREADS_INC_UDP_THREADS_INTERFACE_H_

#include "thread_center_interface.h"

class ConfCenterInterface;
class MsgCodecCenterInterface;

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
struct ThreadsCtx
{
    int argc;
    char** argv;
    const char* common_component_dir;
    const char* cur_working_dir;
    const char* app_name;
    ConfCenterInterface* conf_center;
    MsgCodecCenterInterface* msg_codec_center;
    ThreadCenterInterface* thread_center;

    app_frame::ConfMgrInterface* conf_mgr;

    int* frame_threads_count;
    pthread_mutex_t* frame_threads_mutex;
    pthread_cond_t* frame_threads_cond;

    ThreadsCtx()
    {
        argc = 0;
        argv = nullptr;
        common_component_dir = nullptr;
        cur_working_dir = nullptr;
        app_name = nullptr;
        conf_center = nullptr;
        msg_codec_center = nullptr;
        thread_center = nullptr;
        conf_mgr = nullptr;
        frame_threads_count = nullptr;
        frame_threads_mutex = nullptr;
        frame_threads_cond = nullptr;
    }
};

struct RelatedThreadGroup
{
    ThreadInterface* global_thread;
    global::LogicInterface* global_logic;
    ThreadGroupInterface* work_thread_group;

    RelatedThreadGroup()
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

    virtual int CreateThreadGroup() = 0;
    virtual ThreadGroupInterface* GetUdpThreadGroup() const = 0;

    virtual void SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group) = 0;
};
}

#endif // UDP_THREADS_INC_UDP_THREADS_INTERFACE_H_
