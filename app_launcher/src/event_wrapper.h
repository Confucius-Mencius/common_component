#ifndef APP_LAUNCHER_SRC_EVENT_WRAPPER_H_
#define APP_LAUNCHER_SRC_EVENT_WRAPPER_H_

#include <event2/event.h>
#include "conf_center_interface.h"
#include "file_util.h"
#include "last_err_msg.h"

namespace app_launcher
{
class AppLauncher;

class EventWrapper
{
private:
    static void OnInotifyReadEvent(evutil_socket_t fd, short events, void* arg);
    static void OnStopSignal(evutil_socket_t fd, short events, void* arg);
    static void OnExitCheckTimer(evutil_socket_t fd, short events, void* arg);

public:
    EventWrapper();
    ~EventWrapper();

    const char* GetLastErrMsg() const;
    int Initialize(AppLauncher* app_launcher);
    void Finalize();

    void OnStop();

private:
    int WatchConfFiles();
    int SetStopSignal(int signo);

private:
    LastErrMsg last_err_msg_;
    AppLauncher* app_launcher_;

    int inotify_fd_;
    int inotify_wd_app_conf_;
    int inotify_wd_log_conf_;
    struct event* inotify_read_event_;

    FileStat app_conf_filestat_;
    FileStat log_conf_filestat_;

    struct event* stop_event_;

    // 收到退出信号后，定时检查各个线程是否都成功退出，然后主线程才退出
    bool stopping_;
    struct event* exit_check_timer_event_;
};
}

#endif // APP_LAUNCHER_SRC_EVENT_WRAPPER_H_
