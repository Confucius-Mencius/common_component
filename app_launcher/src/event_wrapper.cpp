#include "event_wrapper.h"
#include <signal.h>
#include <iomanip>
#include <sys/inotify.h>
#include <unistd.h>
#include "app_launcher.h"

namespace app_launcher
{
static const uint32_t inotify_watch_mask =
    IN_CLOSE_WRITE | IN_CREATE | IN_DELETE | IN_DELETE_SELF | IN_MOVE | IN_MOVE_SELF;

void EventWrapper::OnInotifyReadEvent(evutil_socket_t fd, short events, void* arg)
{
    LOG_TRACE("EventWrapper::OnInotifyReadEvent, fd: " << fd
              << ", events: " << setiosflags(std::ios::showbase) << std::hex << events);

    EventWrapper* event_wrapper = static_cast<EventWrapper*>(arg);
    AppLauncher* app_launcher = event_wrapper->app_launcher_;
    bool app_conf_changed = false;
    bool log_conf_changed = false;

    while (true)
    {
        // 读取事件
        char buf[4096] __attribute__ ((aligned(__alignof__(struct inotify_event))));
        ssize_t n = read(fd, buf, sizeof(buf));

        if (0 == n)
        {
            break;
        }
        else if (n < 0)
        {
            const int err = errno;

            if (EINTR == err)
            {
                // 被信号中断了，可以继续读
                continue;
            }
            else if (EAGAIN == err || EWOULDBLOCK == err)
            {
                // 没有数据可读了
                break;
            }
            else
            {
                LOG_ERROR("read error, n: " << n << ", fd: " << fd << ", errno: " << err
                          << ", err msg: " << strerror(err));
                break;
            }
        }

        const struct inotify_event* ievent;
        bool need_check_filestat = false;
        bool need_rewatch = false;

        for (char* ptr = buf; ptr < buf + n;
                ptr += sizeof(struct inotify_event) + ievent->len)
        {
            ievent = (const struct inotify_event*) ptr;

            LOG_DEBUG("inotify mask: " << setiosflags(std::ios::showbase) << std::hex << ievent->mask
                      << ", len: " << ievent->len << ", wd: " << ievent->wd);

            if (ievent->len > 0)
            {
                LOG_DEBUG("inotify event name: " << ievent->name);
            }

            if (ievent->mask & IN_CLOSE_WRITE) // 需要监听，且要比较file stat是否有变化
            {
                LOG_TRACE("IN_CLOSE_WRITE");
                need_check_filestat = true;
            }
            else if (ievent->mask & (IN_DELETE | IN_DELETE_SELF)) // 需要监听，需要rm后再add watch
            {
                LOG_TRACE("IN_DELETE | IN_DELETE_SELF");
                need_check_filestat = true;
                need_rewatch = true;
            }

            if (need_check_filestat)
            {
                const AppLauncherCtx* app_launcher_ctx = event_wrapper->app_launcher_->GetAppLauncherCtx();

                if (ievent->wd == event_wrapper->inotify_wd_app_conf_)
                {
                    FileStat app_conf_filestat;
                    GetFileStat(app_conf_filestat, app_launcher_ctx->app_conf_file_path);

                    if (!app_conf_filestat.Equals(event_wrapper->app_conf_filestat_))
                    {
                        app_conf_changed = true;
                        event_wrapper->app_conf_filestat_ = app_conf_filestat;
                    }
                }

                if (ievent->wd == event_wrapper->inotify_wd_log_conf_)
                {
                    FileStat log_conf_filestat;
                    GetFileStat(log_conf_filestat, app_launcher_ctx->log_conf_file_path);

                    if (!log_conf_filestat.Equals(event_wrapper->log_conf_filestat_))
                    {
                        log_conf_changed = true;
                        event_wrapper->log_conf_filestat_ = log_conf_filestat;
                    }
                }
            }

            if (need_rewatch)
            {
                if (ievent->wd == event_wrapper->inotify_wd_app_conf_)
                {
                    app_conf_changed = true;

                    inotify_rm_watch(event_wrapper->inotify_fd_, event_wrapper->inotify_wd_app_conf_);

                    event_wrapper->inotify_wd_app_conf_ = inotify_add_watch(event_wrapper->inotify_fd_,
                                                          event_wrapper->app_launcher_->GetAppLauncherCtx()->app_conf_file_path,
                                                          inotify_watch_mask);
                    if (-1 == event_wrapper->inotify_wd_app_conf_)
                    {
                        const int err = errno;
                        LOG_ERROR("inotify_add_watch failed (app conf), errno: " << err << ", err msg: " << strerror((err)));
                    }

                    LOG_DEBUG("inotify wd of app conf: " << event_wrapper->inotify_wd_app_conf_);
                }

                if (ievent->wd == event_wrapper->inotify_wd_log_conf_)
                {
                    log_conf_changed = true;

                    inotify_rm_watch(event_wrapper->inotify_fd_, event_wrapper->inotify_wd_log_conf_);

                    event_wrapper->inotify_wd_log_conf_ = inotify_add_watch(event_wrapper->inotify_fd_,
                                                          event_wrapper->app_launcher_->GetAppLauncherCtx()->log_conf_file_path,
                                                          inotify_watch_mask);
                    if (-1 == event_wrapper->inotify_wd_log_conf_)
                    {
                        const int err = errno;
                        LOG_ERROR("inotify_add_watch failed (log conf), errno: " << err << ", err msg: " << strerror((err)));
                    }

                    LOG_DEBUG("inotify wd of log conf: " << event_wrapper->inotify_wd_log_conf_);
                }
            }
        }
    }

    LOG_DEBUG("app conf changed: " << app_conf_changed << ", log conf changed: " << log_conf_changed);
    app_launcher->OnReload(app_conf_changed, log_conf_changed);
}

void EventWrapper::OnStopSignal(evutil_socket_t fd, short events, void* arg)
{
    LOG_ALWAYS("receive stop signal");
    EventWrapper* event_wrapper = static_cast<EventWrapper*>(arg);
    event_wrapper->OnStop();
}

void EventWrapper::OnExitCheckTimer(evutil_socket_t fd, short events, void* arg)
{
    LOG_TRACE("in exit check timer");
    AppLauncher* app_launcher = static_cast<AppLauncher*>(arg);
    app_launcher->OnExitCheck();
}

EventWrapper::EventWrapper() : last_err_msg_(), app_conf_filestat_(), log_conf_filestat_()
{
    app_launcher_ = nullptr;
    inotify_fd_ = -1;
    inotify_wd_app_conf_ = -1;
    inotify_wd_log_conf_ = -1;
    inotify_read_event_ = nullptr;
    stop_event_ = nullptr;
    stopping_ = false;
    exit_check_timer_event_ = nullptr;
}

EventWrapper::~EventWrapper()
{
}

const char* EventWrapper::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

int EventWrapper::Initialize()
{
    // 要屏蔽的信号
    // Block some signals; other threads created by main() will inherit a copy of the signal mask.
    sigset_t set;

    sigemptyset(&set);
    // 要屏蔽的信号
//    sigaddset(&set, SIGINT); // 如果屏蔽了该信号，gdb中按CTRL+C不能中断程序打断点。
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGPIPE); // 忽略SIGPIPE，write时若对端关闭，返回值为-1，errno为EPIPE

    pthread_sigmask(SIG_BLOCK, &set, nullptr);

    // 监控配置文件的变化
    if (WatchConfFiles() != 0)
    {
        return -1;
    }

    if (SetStopSignal(SIGQUIT) != 0) // 3
    {
        return -1;
    }

    return 0;
}

void EventWrapper::Finalize()
{
    if (exit_check_timer_event_ != nullptr)
    {
        event_del(exit_check_timer_event_);
        event_free(exit_check_timer_event_);
        exit_check_timer_event_ = nullptr;
    }

    if (inotify_read_event_ != nullptr)
    {
        event_del(inotify_read_event_);
        event_free(inotify_read_event_);
        inotify_read_event_ = nullptr;
    }

    if (inotify_fd_ != -1)
    {
        if (inotify_wd_log_conf_ != -1)
        {
            inotify_rm_watch(inotify_fd_, inotify_wd_log_conf_);
            inotify_wd_log_conf_ = -1;
        }

        if (inotify_wd_app_conf_ != -1)
        {
            inotify_rm_watch(inotify_fd_, inotify_wd_app_conf_);
            inotify_wd_app_conf_ = -1;
        }

        close(inotify_fd_);
        inotify_fd_ = -1;
    }

    if (stop_event_ != nullptr)
    {
        event_del(stop_event_);
        event_free(stop_event_);
        stop_event_ = nullptr;
    }
}

void EventWrapper::OnStop()
{
    if (stopping_ || nullptr == app_launcher_->GetThreadEvBase())
    {
        return;
    }

    stopping_ = true;

    AppFrameInterface* app_frame = app_launcher_->GetAppFrame();
    if (app_frame != nullptr)
    {
        app_frame->NotifyStop();
    }

    exit_check_timer_event_ = event_new(app_launcher_->GetThreadEvBase(), -1, EV_PERSIST,
                                        EventWrapper::OnExitCheckTimer, app_launcher_);
    if (nullptr == exit_check_timer_event_)
    {
        const int err = errno;
        LOG_ERROR("failed to create exit check timer event, errno: " << err
                  << ", err msg: " << strerror(err));
        return;
    }

    // 起一个退出检测定时器，100毫秒检查一次是否可以退出
    const struct timeval tv = {0, 100000};

    if (event_add(exit_check_timer_event_, &tv) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to add exit check timer event, errno: " << err
                  << ", err msg: " << strerror(err));
        return;
    }
}

int EventWrapper::WatchConfFiles()
{
    inotify_fd_ = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (-1 == inotify_fd_)
    {
        const int err = errno;
        LOG_ERROR("inotify_init1 failed, errno: " << err << ", err msg: " << strerror((err)));
        return -1;
    }

    LOG_DEBUG("inotify fd: " << inotify_fd_);

    const AppLauncherCtx* app_launcher_ctx = app_launcher_->GetAppLauncherCtx();

    inotify_wd_app_conf_ = inotify_add_watch(inotify_fd_, app_launcher_ctx->app_conf_file_path, inotify_watch_mask);
    if (-1 == inotify_wd_app_conf_)
    {
        const int err = errno;
        LOG_ERROR("inotify_add_watch failed, errno: " << err << ", err msg: " << strerror((err)));
        return -1;
    }

    LOG_DEBUG("inotify wd of app conf: " << inotify_wd_app_conf_);

    inotify_wd_log_conf_ = inotify_add_watch(inotify_fd_, app_launcher_ctx->log_conf_file_path, inotify_watch_mask);
    if (-1 == inotify_wd_log_conf_)
    {
        const int err = errno;
        LOG_ERROR("inotify_add_watch failed, errno: " << err << ", err msg: " << strerror((err)));
        return -1;
    }

    LOG_DEBUG("inotify wd of log conf: " << inotify_wd_log_conf_);

    inotify_read_event_ = event_new(app_launcher_->GetThreadEvBase(), inotify_fd_, EV_READ | EV_PERSIST,
                                    EventWrapper::OnInotifyReadEvent, this);
    if (nullptr == inotify_read_event_)
    {
        const int err = errno;
        LOG_ERROR("failed to create inotify event, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (event_add(inotify_read_event_, nullptr) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to add inotify event, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    GetFileStat(app_conf_filestat_, app_launcher_ctx->app_conf_file_path);
    GetFileStat(log_conf_filestat_, app_launcher_ctx->log_conf_file_path);

    return 0;
}

int EventWrapper::SetStopSignal(int signo)
{
    stop_event_ = event_new(app_launcher_->GetThreadEvBase(), signo, EV_SIGNAL | EV_PERSIST,
                            EventWrapper::OnStopSignal, this);
    if (nullptr == stop_event_)
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to create stop signal event, errno: "
                         << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (event_add(stop_event_, nullptr) != 0)
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to add stop signal event, errno: "
                         << err << ", err msg: " << strerror(err));
        return -1;
    }

    return 0;
}
}
