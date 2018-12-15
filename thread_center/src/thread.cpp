#include "thread.h"
#include <fcntl.h>
#include <unistd.h>
#include "file_util.h"
#include "str_util.h"

//In Linux versions before 2.6.11, the capacity of a pipe was the same as the system page size (e.g., 4096 bytes on i386).
//Since Linux 2.6.11, the pipe capacity is 65536 bytes.
//Since Linux 2.6.35, the default pipe capacity is 65536 bytes,
//but the capacity can be queried and set using the fcntl(2) F_GETPIPE_SZ and F_SETPIPE_SZ operations.
//cat /proc/sys/fs/pipe-max-size可以查看当前的最大值，Linux上是1M

namespace thread_center
{
enum
{
    PENDING_NOTIFY_TIMER_ID = 1,
};

void Thread::OnEvent(int fd, short which, void* arg)
{
    Thread* thread = (Thread*) arg;

    do
    {
        char buf[1] = "";
        if (read(fd, buf, 1) != 1)
        {
            const int err = errno;
            if (EINTR == err)
            {
                continue;
            }

            // write了才会触发read，且每次只read一次，所以不会有EAGAIN
            // 真正出错了
            LOG_ERROR("failed to read pipe, errno: " << err << ", err msg: " << strerror(err));
            return;
        }

        switch (buf[0])
        {
            case 's':
            {
                thread->OnStop();
            }
            break;

            case 'r':
            {
                thread->OnReload();
            }
            break;

            case 'e':
            {
                thread->OnExit();
            }
            break;

            case 't':
            {
                thread->OnTask();
            }
            break;

            default:
            {
                LOG_ERROR("invalid cmd: " << buf[0]);
            }
            break;
        }
    } while (0);
}

void* Thread::ThreadRoutine(void* arg)
{
    Thread* thread = (Thread*) arg;
    return thread->WorkLoop();
}

Thread::Thread() : thread_ctx_(), write_fd_mutex_(), tq_(), timer_axis_loader_(), pending_notify_list_()
{
    thread_id_ = (pthread_t) -1;
    thread_ev_base_ = NULL;
    pipe_[0] = pipe_[1] = -1;
    event_ = NULL;
    stopping_ = false;
    timer_axis_ = NULL;
}

Thread::~Thread()
{
}

void Thread::Release()
{
    SAFE_RELEASE_MODULE(timer_axis_, timer_axis_loader_);
    tq_.Release();
    delete this;
}

int Thread::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    thread_ctx_ = *((const ThreadCtx*) ctx);
    tq_.SetThread(this);

    thread_ev_base_ = event_base_new();
    if (NULL == thread_ev_base_)
    {
        const int err = errno;
        LOG_ERROR("failed to create thread event base, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    //  pipefd[0] refers to the read end of the pipe.  pipefd[1] refers to the write end of the pipe.
    if (pipe2(pipe_, O_CLOEXEC | O_NONBLOCK) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to create pipe, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    // 默认是65536
    int pipe0_size = fcntl(pipe_[0], F_GETPIPE_SZ);
    int pipe1_size = fcntl(pipe_[1], F_GETPIPE_SZ);
    LOG_DEBUG("before set, pipe0 size: " << pipe0_size << ", pipe1 size: " << pipe1_size);

    // 修改为1048576。超过最大值时返回-1，设置失败
    const int pipe_size = 1048576;
    if (-1 == fcntl(pipe_[1], F_SETPIPE_SZ, pipe_size))
    {
        const int err = errno;
        LOG_ERROR("failed to set pipe size to " << pipe_size << ", errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    pipe0_size = fcntl(pipe_[0], F_GETPIPE_SZ);
    pipe1_size = fcntl(pipe_[1], F_GETPIPE_SZ);
    LOG_DEBUG("after set, pipe0 size: " << pipe0_size << ", pipe1 size: " << pipe1_size);

    event_ = event_new(thread_ev_base_, pipe_[0], EV_READ | EV_PERSIST, Thread::OnEvent, this);
    if (NULL == event_)
    {
        const int err = errno;
        LOG_ERROR("failed to create event, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (event_add(event_, 0) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to add event, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (LoadTimerAxis() != 0)
    {
        return -1;
    }

    if (thread_ctx_.sink->OnInitialize(this) != 0)
    {
        return -1;
    }

    return 0;
}

void Thread::Finalize()
{
    thread_ctx_.sink->OnFinalize();
    SAFE_FINALIZE(timer_axis_);

    if (event_ != NULL)
    {
        event_del(event_);
        event_free(event_);
        event_ = NULL;
    }

    if (pipe_[0] != -1)
    {
        close(pipe_[0]);
    }

    if (pipe_[1] != -1)
    {
        close(pipe_[1]);
    }

    if (thread_ev_base_ != NULL)
    {
        event_base_free(thread_ev_base_);
        thread_ev_base_ = NULL;
    }

    pending_notify_list_.clear();
}

int Thread::Activate()
{
    if (SAFE_ACTIVATE_FAILED(timer_axis_))
    {
        return -1;
    }

    if (thread_ctx_.sink->OnActivate() != 0)
    {
        return -1;
    }

    return 0;
}

void Thread::Freeze()
{
    StopPendingNotifyTimer();
    thread_ctx_.sink->OnFreeze();
    SAFE_FREEZE(timer_axis_);
}

void Thread::PushTask(Task* task)
{
    ThreadInterface* source_thread = task->GetSourceThread();

    // TODO 这里暂时共用一个tq、pipe、event
    TaskQueue* tq = GetTaskQueue(source_thread);
    const int fd = GetPipeWriteFD(source_thread);

    tq->PushBack(task);
    return NotifyTask(fd);
}

void Thread::OnTimer(TimerID timer_id, void* data, size_t len, int times)
{
    std::lock_guard<std::mutex> lock(write_fd_mutex_);

    if (pending_notify_list_.empty())
    {
        StopPendingNotifyTimer();
        return;
    }

    const char buf[1] = { pending_notify_list_.front() };

    if (1 == write(pipe_[1], buf, 1))
    {
        pending_notify_list_.pop_front();
    }
}

int Thread::Start()
{
    if (pthread_create(&thread_id_, NULL, Thread::ThreadRoutine, this) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to create thread, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    return 0;
}

void Thread::Join()
{
    if (thread_id_ != (pthread_t) -1)
    {
        pthread_join(thread_id_, NULL);
    }
}

void* Thread::WorkLoop()
{
    thread_ctx_.sink->OnThreadStartOK();
    event_base_dispatch(thread_ev_base_);
    pthread_exit((void*) 0);
}

void Thread::NotifyStop()
{
    static const char buf[1] = { 's' };
    std::lock_guard<std::mutex> lock(write_fd_mutex_);

    if (write(pipe_[1], buf, 1) != 1)
    {
        const int err = errno;
        LOG_WARN("failed to write pipe, errno: " << err << ", err msg: " << strerror(err));

        pending_notify_list_.push_back('s');
        StartPendingNotifyTimer();
    }
}

void Thread::OnStop()
{
    if (stopping_)
    {
        return;
    }

    stopping_ = true;

    while (!tq_.IsEmpty())
    {
        Task* task = tq_.PopFront();
        if (task != NULL)
        {
            thread_ctx_.sink->OnTask(task);
            task->Release();
        }
    }

    thread_ctx_.sink->OnStop();
}

void Thread::NotifyReload()
{
    static const char buf[1] = { 'r' };
    std::lock_guard<std::mutex> lock(write_fd_mutex_);

    if (write(pipe_[1], buf, 1) != 1)
    {
        const int err = errno;
        LOG_WARN("failed to write pipe, errno: " << err << ", err msg: " << strerror(err));

        pending_notify_list_.push_back('r');
        StartPendingNotifyTimer();
    }
}

void Thread::OnReload()
{
    thread_ctx_.sink->OnReload();
}

bool Thread::CanExit() const
{
    return thread_ctx_.sink->CanExit();
}

void Thread::NotifyExit()
{
    static const char buf[1] = { 'e' };
    std::lock_guard<std::mutex> lock(write_fd_mutex_);

    if (write(pipe_[1], buf, 1) != 1)
    {
        const int err = errno;
        LOG_WARN("failed to write pipe, errno: " << err << ", err msg: " << strerror(err));

        pending_notify_list_.push_back('e');
        StartPendingNotifyTimer();
    }
}

void Thread::OnExit()
{
    event_base_loopbreak(thread_ev_base_);
}

void Thread::NotifyTask(int fd)
{
    static const char buf[1] = { 't' };
    std::lock_guard<std::mutex> lock(write_fd_mutex_);

    // man 7 pipe:
    // O_NONBLOCK disabled, n <= PIPE_BUF (PIPE_BUF可通过ulimit -p查看，Linux上是4096字节)
    //     All n bytes are written atomically; write(2) may block if there is not room for n bytes to be written immediately
    // O_NONBLOCK enabled, n <= PIPE_BUF
    //     If there is room to write n bytes to the pipe, then write(2) succeeds immediately, writing all n bytes; otherwise write(2) fails, with errno set to EAGAIN.
    if (write(fd, buf, 1) != 1)
    {
        // 被信号中断或者pipe满了
        const int err = errno;
        LOG_WARN("failed to write pipe, errno: " << err << ", err msg: " << strerror(err));

        pending_notify_list_.push_back('t');
        StartPendingNotifyTimer();
    }
}

void Thread::OnTask()
{
    Task* task = tq_.PopFront();
    if (task != NULL)
    {
        thread_ctx_.sink->OnTask(task);
        task->Release();
    }
}

int Thread::LoadTimerAxis()
{
    char TIMER_AXIS_SO_PATH[MAX_PATH_LEN + 1] = "";
    StrPrintf(TIMER_AXIS_SO_PATH, sizeof(TIMER_AXIS_SO_PATH), "%s/libtimer_axis.so",
              thread_ctx_.common_component_dir);

    if (timer_axis_loader_.Load(TIMER_AXIS_SO_PATH) != 0)
    {
        LOG_ERROR(timer_axis_loader_.GetLastErrMsg());
        return -1;
    }

    timer_axis_ = (TimerAxisInterface*) timer_axis_loader_.GetModuleInterface();
    if (NULL == timer_axis_)
    {
        LOG_ERROR(timer_axis_loader_.GetLastErrMsg());
        return -1;
    }

    TimerAxisCtx timer_axis_ctx;
    timer_axis_ctx.thread_ev_base = thread_ev_base_;

    if (timer_axis_->Initialize(&timer_axis_ctx) != 0)
    {
        LOG_ERROR("failed to initialize timer axis");
        return -1;
    }

    return 0;
}

void Thread::StartPendingNotifyTimer()
{
    if (NULL == timer_axis_)
    {
        return;
    }

    if (timer_axis_->TimerExist(this, PENDING_NOTIFY_TIMER_ID))
    {
        return;
    }

    struct timeval interval = {0, 1};
    timer_axis_->SetTimer(this, PENDING_NOTIFY_TIMER_ID, interval);
}

void Thread::StopPendingNotifyTimer()
{
    if (NULL == timer_axis_)
    {
        return;
    }

    timer_axis_->KillTimer(this, PENDING_NOTIFY_TIMER_ID);
}
} // namespace thread_center
