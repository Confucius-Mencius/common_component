#include "signal_wrapper.h"
#include <signal.h>
#include "app_launcher.h"

namespace app_launcher
{
void SignalWrapper::OnStopSignal(evutil_socket_t fd, short event, void* arg)
{
    (void) fd;
    (void) event;

    LOG_ALWAYS("receive stop signal");
    AppLauncher* app_launcher = static_cast<AppLauncher*>(arg);
    app_launcher->OnStop();
}

void SignalWrapper::OnReloadSignal(evutil_socket_t fd, short event, void* arg)
{
    (void) fd;
    (void) event;

    LOG_ALWAYS("receive reload signal");
    AppLauncher* app_launcher = static_cast<AppLauncher*>(arg);
    app_launcher->OnReload();
}

SignalWrapper::SignalWrapper() : last_err_msg_()
{
    app_launcher_ = NULL;
    stop_event_ = NULL;
    reload_event_ = NULL;
}

SignalWrapper::~SignalWrapper()
{
}

const char* SignalWrapper::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

int SignalWrapper::Initialize(AppLauncher* app_launcher)
{
    if (NULL == app_launcher)
    {
        return -1;
    }

    app_launcher_ = app_launcher;

    // 要屏蔽的信号，可以做成配置
//    int ignore_signals[] = {SIGINT, SIGTERM, SIGPIPE};

//    for (int i = 0; i < COUNT_OF(ignore_signals); ++i)
//    {
//        if (IgnoreSignal(ignore_signals[i]) != 0)
//        {
//            return -1;
//        }
//    }

    // Block some signals; other threads created by main() will inherit a copy of the signal mask.
    sigset_t set;

    sigemptyset(&set);
    // 要屏蔽的信号
//    sigaddset(&set, SIGINT); // 如果屏蔽了该信号，gdb中按CTRL+C不能中断程序打断点。
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGPIPE); // 忽略SIGPIPE，write时若对端关闭，返回值为-1，errno为EPIPE

    pthread_sigmask(SIG_BLOCK, &set, NULL);

    if (SetStopSignal(SIGQUIT) != 0) // 3
    {
        return -1;
    }

    if (SetReloadSignal(SIGHUP) != 0) // 1
    {
        return -1;
    }

    return 0;
}

void SignalWrapper::Finalize()
{
    if (reload_event_ != NULL)
    {
        event_del(reload_event_);
        event_free(reload_event_);
        reload_event_ = NULL;
    }

    if (stop_event_ != NULL)
    {
        event_del(stop_event_);
        event_free(stop_event_);
        stop_event_ = NULL;
    }
}

//int SignalWrapper::IgnoreSignal(int signo)
//{
//    struct sigaction sa;

//    sa.sa_handler = SIG_IGN;
//    sigemptyset(&sa.sa_mask);
//    sa.sa_flags = 0;

//    if (sigaction(signo, &sa, NULL) != 0)
//    {
//        const int err = errno;
//        SET_LAST_ERR_MSG(&last_err_msg_, "sigaction failed, errno: " << err << ", err msg: " << strerror(err));
//        return -1;
//    }

//    return 0;
//}

int SignalWrapper::SetStopSignal(int signo)
{
    stop_event_ = event_new(app_launcher_->GetThreadEvBase(), signo, EV_SIGNAL | EV_PERSIST,
                            SignalWrapper::OnStopSignal, app_launcher_);
    if (NULL == stop_event_)
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to create stop signal event, errno: "
                         << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (event_add(stop_event_, NULL) != 0)
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to add stop signal event, errno: "
                         << err << ", err msg: " << strerror(err));
        return -1;
    }

    return 0;
}

int SignalWrapper::SetReloadSignal(int signo)
{
    reload_event_ = event_new(app_launcher_->GetThreadEvBase(), signo, EV_SIGNAL | EV_PERSIST,
                              SignalWrapper::OnReloadSignal, app_launcher_);
    if (NULL == reload_event_)
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to create reload signal event, errno: "
                         << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (event_add(reload_event_, NULL) != 0)
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to add reload conf signal event, errno: "
                         << err << ", err msg: " << strerror(err));
        return -1;
    }

    return 0;
}
}
