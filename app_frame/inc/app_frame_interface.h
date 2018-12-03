#ifndef APP_FRAME_INC_APP_FRAME_INTERFACE_H_
#define APP_FRAME_INC_APP_FRAME_INTERFACE_H_

#include "common_define.h"
#include "module_interface.h"

class ConfCenterInterface;
class MsgCodecCenterInterface;
class ThreadCenterInterface;

struct AppFrameCtx
{
    int argc;
    char** argv;
    const char* common_component_dir;
    const char* cur_working_dir;
    const char* app_name;
    ConfCenterInterface* conf_center;
    MsgCodecCenterInterface* msg_codec_center;
    ThreadCenterInterface* thread_center;

    AppFrameCtx()
    {
        argc = 0;
        argv = NULL;
        common_component_dir = NULL;
        cur_working_dir = NULL;
        app_name = NULL;
        conf_center = NULL;
        msg_codec_center = NULL;
        thread_center = NULL;
    }
};

class AppFrameInterface : public ModuleInterface
{
public:
    virtual ~AppFrameInterface()
    {
    }

    virtual int NotifyStop() = 0;
    virtual int NotifyReload(bool changed) = 0;

    virtual bool CanExit() const = 0;
    virtual int NotifyExit() = 0;
};

#endif // APP_FRAME_INC_APP_FRAME_INTERFACE_H_
