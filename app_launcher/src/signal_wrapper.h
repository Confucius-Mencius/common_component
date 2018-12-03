#ifndef APP_LAUNCHER_SRC_SIGNAL_WRAPPER_H_
#define APP_LAUNCHER_SRC_SIGNAL_WRAPPER_H_

#include <event2/event.h>
#include "conf_center_interface.h"
#include "last_err_msg.h"

namespace app_launcher
{
class AppLauncher;

class SignalWrapper
{
public:
    static void OnStopSignal(evutil_socket_t fd, short event, void* arg);
    static void OnReloadSignal(evutil_socket_t fd, short event, void* arg);

public:
    SignalWrapper();
    ~SignalWrapper();

    const char* GetLastErrMsg() const;
    int Initialize(AppLauncher* app_launcher);
    void Finalize();

private:
//    int IgnoreSignal(int signo);
    int SetStopSignal(int signo);
    int SetReloadSignal(int signo);

private:
    LastErrMsg last_err_msg_;
    AppLauncher* app_launcher_;
    struct event* stop_event_;
    struct event* reload_event_;
};
}

#endif // APP_LAUNCHER_SRC_SIGNAL_WRAPPER_H_
