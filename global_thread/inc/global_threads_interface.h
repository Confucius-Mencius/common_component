#ifndef GLOBAL_THREAD_INC_GLOBAL_THREADS_INTERFACE_H_
#define GLOBAL_THREAD_INC_GLOBAL_THREADS_INTERFACE_H_

#include "thread_center_interface.h"

class ConfCenterInterface;
class MsgCodecCenterInterface;

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
    ThreadGroupInterface* burden_thread_group;
    ThreadGroupInterface* work_thread_group;
    ThreadGroupInterface* tcp_thread_group;
    ThreadGroupInterface* http_thread_group;
    ThreadGroupInterface* udp_thread_group;

    RelatedThreadGroup()
    {
        burden_thread_group = NULL;
        work_thread_group = NULL;
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

    virtual ThreadGroupInterface* GetGlobalThreadGroup() const = 0;
    virtual LogicInterface* GetLogic() const = 0; // global logic

    virtual void SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group) = 0;

    virtual void SetReloadFinish(bool finish) = 0;
    virtual bool ReloadFinished() = 0;
};
}

#endif // GLOBAL_THREAD_INC_GLOBAL_THREADS_INTERFACE_H_
