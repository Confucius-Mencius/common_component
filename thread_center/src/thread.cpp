#include "thread.h"
#include <unistd.h>
#include "scheduler_interface.h"
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

Thread::Thread() : thread_ctx_(), write_fd_mutex_(), tq_(), timer_axis_loader_(), time_service_loader_(),
                   random_engine_loader_(), trans_center_loader_(), conn_center_mgr_loader_(),
                   client_center_mgr_loader_(), msg_dispatcher_(), normal_msg_dispatcher_()
{
    thread_id_ = (pthread_t) -1;
    thread_ev_base_ = NULL;
    pipe_[0] = pipe_[1] = -1;
    event_ = NULL;
    stopping_ = false;
    timer_axis_ = NULL;
    time_service_ = NULL;
    random_engine_ = NULL;
    trans_center_ = NULL;
    conn_center_mgr_ = NULL;
    client_center_mgr_ = NULL;
}

Thread::~Thread()
{
}

void Thread::Release()
{
    SAFE_RELEASE_MODULE(client_center_mgr_, client_center_mgr_loader_);
    SAFE_RELEASE_MODULE(conn_center_mgr_, conn_center_mgr_loader_);
    SAFE_RELEASE_MODULE(trans_center_, trans_center_loader_);
    SAFE_RELEASE_MODULE(random_engine_, random_engine_loader_);
    SAFE_RELEASE_MODULE(time_service_, time_service_loader_);
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

//    LOG_INFO("libevent version: " << event_get_version() << ", libevent method: " << event_base_get_method(thread_ev_base_));

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

    if (LoadTimeService() != 0)
    {
        return -1;
    }

    if (LoadRandomEngine() != 0)
    {
        return -1;
    }

    if (LoadTransCenter() != 0)
    {
        return -1;
    }

    if (LoadConnCenterMgr() != 0)
    {
        return -1;
    }

    if (LoadClientCenterMgr() != 0)
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

    SAFE_FINALIZE(client_center_mgr_);
    SAFE_FINALIZE(conn_center_mgr_);
    SAFE_FINALIZE(trans_center_);
    SAFE_FINALIZE(random_engine_);
    SAFE_FINALIZE(time_service_);
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

    if (SAFE_ACTIVATE_FAILED(time_service_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(random_engine_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(trans_center_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(conn_center_mgr_))
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(client_center_mgr_))
    {
        return -1;
    }

    if (thread_ctx_.sink->OnActivate() != 0)
    {
        return -1;
    }

    if (pthread_create(&thread_id_, NULL, Thread::ThreadRoutine, this) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to create thread, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    return 0;
}

// TODO 增加一对start和join接口，start在activate之后调用，join在freeze之前调用

void Thread::Freeze()
{
    thread_ctx_.sink->OnFreeze();

    SAFE_FREEZE(client_center_mgr_);
    SAFE_FREEZE(conn_center_mgr_);
    SAFE_FREEZE(trans_center_);
    SAFE_FREEZE(random_engine_);
    SAFE_FREEZE(time_service_);
    SAFE_FREEZE(timer_axis_);

    if (thread_id_ != (pthread_t) -1)
    {
        pthread_join(thread_id_, NULL);
    }
}

int Thread::PushTask(Task* task)
{
    ThreadInterface* source_thread = task->GetCtx()->source_thread;

    // TODO 这里暂时共用一个tq、pipe、event
    TaskQueue* tq = GetTaskQueue(source_thread);
    const int fd = GetPipeWriteFD(source_thread);

    tq->PushBack(task);
    return NotifyTask(fd);
}

//TransId Thread::ScheduleTask(Task* task, const base::AsyncParams* params, ThreadInterface* target_thread)
//{
//    TransId trans_id = OK_TRANS_ID;
//
//    if (params != NULL)
//    {
//        TransCtx trans_ctx;
//        trans_ctx.peer.type = PEER_TYPE_THREAD;
//        trans_ctx.peer.addr = target_thread->GetThreadName(); // thread内部专用地址和端口
//        trans_ctx.peer.port = target_thread->GetThreadIdx();
//
//        trans_ctx.timeout_sec = params->timeout_sec;
//        trans_ctx.passback = task->GetCtx()->msg_head.passback;
//        trans_ctx.sink = params->sink;
//        trans_ctx.async_data = (char*) params->async_data;
//        trans_ctx.async_data_len = params->async_data_len;
//
//        trans_id = trans_center_->RecordTransCtx(&trans_ctx);
//        if (INVALID_TRANS_ID == trans_id)
//        {
//            return INVALID_TRANS_ID;
//        }
//    }
//
//    if (target_thread->PushTask(task) != 0)
//    {
//        if (params != NULL)
//        {
//            trans_center_->CancelTrans(trans_id);
//            return INVALID_TRANS_ID;
//        }
//
//        return FAILED_TRANS_ID;
//    }
//
//    return trans_id;
//}

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

int Thread::LoadTimeService()
{
    char TIME_SERVICE_SO_PATH[MAX_PATH_LEN + 1] = "";
    StrPrintf(TIME_SERVICE_SO_PATH, sizeof(TIME_SERVICE_SO_PATH), "%s/libtime_service.so",
              thread_ctx_.common_component_dir);

    if (time_service_loader_.Load(TIME_SERVICE_SO_PATH) != 0)
    {
        LOG_ERROR(time_service_loader_.GetLastErrMsg());
        return -1;
    }

    time_service_ = (TimeServiceInterface*) time_service_loader_.GetModuleInterface();
    if (NULL == time_service_)
    {
        LOG_ERROR(time_service_loader_.GetLastErrMsg());
        return -1;
    }

    if (time_service_->Initialize(NULL) != 0)
    {
        LOG_ERROR("failed to initialize time service");
        return -1;
    }

    return 0;
}

int Thread::LoadRandomEngine()
{
    char RANDOM_ENGINE_SO_PATH[MAX_PATH_LEN + 1] = "";
    StrPrintf(RANDOM_ENGINE_SO_PATH, sizeof(RANDOM_ENGINE_SO_PATH), "%s/librandom_engine.so",
              thread_ctx_.common_component_dir);

    if (random_engine_loader_.Load(RANDOM_ENGINE_SO_PATH) != 0)
    {
        LOG_ERROR(random_engine_loader_.GetLastErrMsg());
        return -1;
    }

    random_engine_ = (RandomEngineInterface*) random_engine_loader_.GetModuleInterface();
    if (NULL == random_engine_)
    {
        LOG_ERROR(random_engine_loader_.GetLastErrMsg());
        return -1;
    }

    if (random_engine_->Initialize(NULL) != 0)
    {
        LOG_ERROR("failed to initialize random engine");
        return -1;
    }

    random_engine_->Seed();
    return 0;
}

int Thread::LoadTransCenter()
{
    char TRANS_CENTER_SO_PATH[MAX_PATH_LEN + 1] = "";
    StrPrintf(TRANS_CENTER_SO_PATH, sizeof(TRANS_CENTER_SO_PATH), "%s/libtrans_center.so",
              thread_ctx_.common_component_dir);

    if (trans_center_loader_.Load(TRANS_CENTER_SO_PATH) != 0)
    {
        LOG_ERROR(trans_center_loader_.GetLastErrMsg());
        return -1;
    }

    trans_center_ = (TransCenterInterface*) trans_center_loader_.GetModuleInterface();
    if (NULL == trans_center_)
    {
        LOG_ERROR(trans_center_loader_.GetLastErrMsg());
        return -1;
    }

    TransCenterCtx trans_center_ctx;
    trans_center_ctx.timer_axis = timer_axis_;
    trans_center_ctx.need_reply_msg_check_interval = thread_ctx_.need_reply_msg_check_interval;

    if (trans_center_->Initialize(&trans_center_ctx) != 0)
    {
        LOG_ERROR("failed to initialize trans center");
        return -1;
    }

    return 0;
}

int Thread::LoadConnCenterMgr()
{
    char CONN_CENTER_MGR_SO_PATH[MAX_PATH_LEN + 1] = "";
    StrPrintf(CONN_CENTER_MGR_SO_PATH, sizeof(CONN_CENTER_MGR_SO_PATH), "%s/libconn_center_mgr.so",
              thread_ctx_.common_component_dir);

    if (conn_center_mgr_loader_.Load(CONN_CENTER_MGR_SO_PATH) != 0)
    {
        LOG_ERROR(conn_center_mgr_loader_.GetLastErrMsg());
        return -1;
    }

    conn_center_mgr_ = (ConnCenterMgrInterface*) conn_center_mgr_loader_.GetModuleInterface();
    if (NULL == conn_center_mgr_)
    {
        LOG_ERROR(conn_center_mgr_loader_.GetLastErrMsg());
        return -1;
    }

    if (conn_center_mgr_->Initialize(NULL) != 0)
    {
        LOG_ERROR("failed to initialize conn center mgr");
        return -1;
    }

    return 0;
}

int Thread::LoadClientCenterMgr()
{
    char CLIENT_CENTER_MGR_SO_PATH[MAX_PATH_LEN + 1] = "";
    StrPrintf(CLIENT_CENTER_MGR_SO_PATH, sizeof(CLIENT_CENTER_MGR_SO_PATH), "%s/libclient_center_mgr.so",
              thread_ctx_.common_component_dir);

    if (client_center_mgr_loader_.Load(CLIENT_CENTER_MGR_SO_PATH) != 0)
    {
        LOG_ERROR(client_center_mgr_loader_.GetLastErrMsg());
        return -1;
    }

    client_center_mgr_ = (ClientCenterMgrInterface*) client_center_mgr_loader_.GetModuleInterface();
    if (NULL == client_center_mgr_)
    {
        LOG_ERROR(client_center_mgr_loader_.GetLastErrMsg());
        return -1;
    }

//    ClientCenterMgrCtx ctx;
//    ctx.common_component_dir = thread_ctx_.common_component_dir;
//    ctx.timer_axis = timer_axis_;
//    ctx.trans_center = trans_center_;

    if (client_center_mgr_->Initialize(NULL) != 0)
    {
        LOG_ERROR("failed to initialize client center mgr");
        return -1;
    }

    return 0;
}
} // namespace thread_center
