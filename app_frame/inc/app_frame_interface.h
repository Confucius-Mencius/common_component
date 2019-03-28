#ifndef APP_FRAME_INC_APP_FRAME_INTERFACE_H_
#define APP_FRAME_INC_APP_FRAME_INTERFACE_H_

#include "module_interface.h"

class ConfCenterInterface;
class ThreadCenterInterface;

struct AppFrameCtx
{
    int argc;
    char** argv;
    const char* common_component_dir;
    const char* cur_working_dir;
    const char* app_name;
    ConfCenterInterface* conf_center;
    ThreadCenterInterface* thread_center;

    AppFrameCtx()
    {
        argc = 0;
        argv = nullptr;
        common_component_dir = nullptr;
        cur_working_dir = nullptr;
        app_name = nullptr;
        conf_center = nullptr;
        thread_center = nullptr;
    }
};

class AppFrameInterface : public ModuleInterface
{
public:
    virtual ~AppFrameInterface()
    {
    }

    virtual int NotifyStop() = 0;
    virtual int NotifyReload() = 0;

    virtual bool CanExit() const = 0;
    virtual int NotifyExitAndJoin() = 0;
};

#endif // APP_FRAME_INC_APP_FRAME_INTERFACE_H_
