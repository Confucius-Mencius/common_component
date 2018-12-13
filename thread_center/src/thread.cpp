#include "thread.h"
#include <unistd.h>
#include "file_util.h"
#include "str_util.h"

namespace thread_center
{
void Thread::OnEvent(int fd, short which, void* arg)
{
    Thread* thread = (Thread*) arg;

    char buf[1] = "";
    if (read(fd, buf, 1) != 1)
    {
        const int err = errno;
        LOG_ERROR("failed to read, errno: " << err << ", err msg: " << strerror(err));
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
        }
        break;
    }
}

void* Thread::ThreadRoutine(void* arg)
{
    Thread* thread = (Thread*) arg;
    return thread->WorkLoop();
}

Thread::Thread() : thread_ctx_(), write_fd_mutex_(), tq_(), timer_axis_loader_()
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

    thread_ev_base_ = event_base_new();
    if (NULL == thread_ev_base_)
    {
        const int err = errno;
        LOG_ERROR("failed to create thread event base, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    if (pipe(pipe_) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to create pipe, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

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
    thread_ctx_.sink->OnFreeze();
    SAFE_FREEZE(timer_axis_);
}

int Thread::PushTask(Task* task)
{
    ThreadInterface* source_thread = task->GetSourceThread();

    // TODO 这里暂时共用一个tq、pipe、event
    TaskQueue* tq = GetTaskQueue(source_thread);
    const int fd = GetPipeWriteFD(source_thread);

    tq->PushBack(task);
    return NotifyTask(fd);
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
    thread_ctx_.sink->OnThreadStartOk();
    event_base_dispatch(thread_ev_base_);
    pthread_exit((void*) 0);
}

int Thread::NotifyStop()
{
    static const char buf[1] = {'s'};
    std::lock_guard<std::mutex> lock(write_fd_mutex_);

    if (write(pipe_[1], buf, 1) != 1)
    {
        const int err = errno;
        LOG_ERROR("failed to write stop notify, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    return 0;
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

int Thread::NotifyReload()
{
    static const char buf[1] = {'r'};
    std::lock_guard<std::mutex> lock(write_fd_mutex_);

    if (write(pipe_[1], buf, 1) != 1)
    {
        const int err = errno;
        LOG_ERROR("failed to write reload notify, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    return 0;
}

void Thread::OnReload()
{
    thread_ctx_.sink->OnReload();
}

bool Thread::CanExit() const
{
    return thread_ctx_.sink->CanExit();
}

int Thread::NotifyExit()
{
    static const char buf[1] = {'e'};
    std::lock_guard<std::mutex> lock(write_fd_mutex_);

    if (write(pipe_[1], buf, 1) != 1)
    {
        const int err = errno;
        LOG_ERROR("failed to write exit notify, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    return 0;
}

void Thread::OnExit()
{
    event_base_loopbreak(thread_ev_base_);
}

int Thread::NotifyTask(int fd)
{
    static const char buf[1] = {'t'};
    std::lock_guard<std::mutex> lock(write_fd_mutex_);

    if (write(fd, buf, 1) != 1)
    {
        const int err = errno;
        LOG_ERROR("failed to write task notify, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    return 0;
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
} // namespace thread_center
