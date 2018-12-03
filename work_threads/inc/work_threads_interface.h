#ifndef WORK_THREADS_INC_WORK_THREADS_INTERFACE_H_
#define WORK_THREADS_INC_WORK_THREADS_INTERFACE_H_

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

namespace work
{
struct ThreadsCtx
{
    int argc;
    char** argv;
    const char* common_component_dir;
    const char* cur_work_dir;
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
        argv = NULL;
        common_component_dir = NULL;
        cur_work_dir = NULL;
        app_name = NULL;
        conf_center = NULL;
        msg_codec_center = NULL;
        thread_center = NULL;
        conf_mgr = NULL;
        frame_threads_count = NULL;
        frame_threads_mutex = NULL;
        frame_threads_cond = NULL;
    }
};

struct RelatedThreadGroup
{
    ThreadInterface* global_thread;
    global::LogicInterface* global_logic;
    ThreadGroupInterface* burden_thread_group;
    ThreadGroupInterface* tcp_thread_group;
    ThreadGroupInterface* http_thread_group;
    ThreadGroupInterface* udp_thread_group;

    RelatedThreadGroup()
    {
        global_thread = NULL;
        global_logic = NULL;
        burden_thread_group = NULL;
        tcp_thread_group = NULL;
        http_thread_group = NULL;
        udp_thread_group = NULL;
    }
};

class ThreadsInterface : public ModuleInterface
{
public:
    virtual ~ThreadsInterface()
    {
    }

    virtual int CreateThreadGroup() = 0;
    virtual ThreadGroupInterface* GetWorkThreadGroup() const = 0;

    virtual void SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group) = 0;
};
}

#endif // WORK_THREADS_INC_WORK_THREADS_INTERFACE_H_
