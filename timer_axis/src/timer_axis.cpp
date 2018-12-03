#include "timer_axis.h"
#include <string.h>
#include "version.h"

namespace timer_axis
{
struct CallbackArg
{
    TimerAxis* timer_axis;
    Timer* timer;

    CallbackArg()
    {
        timer_axis = NULL;
        timer = NULL;
    }

    static CallbackArg* Create(TimerAxis* timer_axis, Timer* timer)
    {
        CallbackArg* callback_arg = new CallbackArg();
        if (NULL == callback_arg)
        {
            const int err = errno;
            SET_LAST_ERR_MSG(&timer_axis->LastErrMsgInstance(), "failed to alloc memory, errno: "
                             << err << ", err msg: " << strerror(err));
            return NULL;
        }

        callback_arg->timer_axis = timer_axis;
        callback_arg->timer = timer;

        return callback_arg;
    }

    void Release()
    {
        delete this;
    }
};

void TimerAxis::OnTimer(evutil_socket_t fd, short event, void* arg)
{
    CallbackArg* callback_arg = (CallbackArg*) arg;
    Timer* timer = callback_arg->timer;
    const int times = ++(timer->times);

    timer->timer_key.sink->OnTimer(timer->timer_key.timer_id, timer->async_data, timer->async_data_len, times);

    if (timer->removed || (timer->total_times > 0 && times >= timer->total_times))
    {
        TimerAxis* timer_axis = callback_arg->timer_axis;
        timer_axis->RemoveTimer(timer);
        callback_arg->Release();
    }
}

TimerAxis::TimerAxis() : last_err_msg_(), timer_axis_ctx_(), timer_hash_map_()
{
}

TimerAxis::~TimerAxis()
{
}

const char* TimerAxis::GetVersion() const
{
    return TIMER_AXIS_TIMER_AXIS_VERSION;
}

const char* TimerAxis::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

void TimerAxis::Release()
{
    delete this;
}

int TimerAxis::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    timer_axis_ctx_ = *((TimerAxisCtx*) ctx);
    return 0;
}

void TimerAxis::Finalize()
{
    for (TimerHashMap::iterator it = timer_hash_map_.begin(); it != timer_hash_map_.end(); ++it)
    {
        Timer& timer = it->second;
        CallbackArg* callback_arg = (CallbackArg*) event_get_callback_arg(timer.event);

        if (timer.async_data != NULL)
        {
            delete[] timer.async_data;
            timer.async_data = NULL;
            timer.async_data_len = 0;
        }

        if (timer.event != NULL)
        {
            event_del(timer.event);
            event_free(timer.event);
            timer.event = NULL;
        }

        callback_arg->Release();
    }

    timer_hash_map_.clear();
}

int TimerAxis::Activate()
{
    return 0;
}

void TimerAxis::Freeze()
{
}

bool TimerAxis::TimerExist(TimerSinkInterface* sink, TimerID timer_id)
{
    if (NULL == sink)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "invalid param");
        return false;
    }

    TimerKey timer_key;
    timer_key.sink = sink;
    timer_key.timer_id = timer_id;

    TimerHashMap::const_iterator it = timer_hash_map_.find(timer_key);
    if (it == timer_hash_map_.end())
    {
        return false;
    }

    return !(it->second.removed);
}

int TimerAxis::SetTimer(TimerSinkInterface* sink, TimerID timer_id, const struct timeval& interval,
                        const void* async_data, size_t async_data_len, int total_times)
{
    if (NULL == sink)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "invalid param");
        return -1;
    }

    TimerKey timer_key;
    timer_key.sink = sink;
    timer_key.timer_id = timer_id;

    TimerHashMap::iterator it = timer_hash_map_.find(timer_key);
    if (it != timer_hash_map_.end())
    {
        Timer& timer = it->second;
        if (!timer.removed)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "timer already exist");
            return -1;
        }

        if (FillAsyncData(&timer, async_data, async_data_len) != 0)
        {
            return -1;
        }

        if (total_times > 0)
        {
            timer.total_times = total_times;
            timer.times = 0;
        }

        if (event_add(timer.event, &interval) != 0)
        {
            const int err = errno;
            SET_LAST_ERR_MSG(&last_err_msg_, "failed to add timer event, errno: "
                             << err << ", err msg: " << strerror(errno));
            ReleaseAsyncData(&timer);
            return -1;
        }

        timer.removed = false;
    }
    else
    {
        // 添加新的
        Timer& timer = timer_hash_map_[timer_key];

        if (FillAsyncData(&timer, async_data, async_data_len) != 0)
        {
            timer_hash_map_.erase(timer_key);
            return -1;
        }

        if (total_times > 0)
        {
            timer.total_times = total_times;
        }

        CallbackArg* callback_arg = CallbackArg::Create(this, &timer);
        if (NULL == callback_arg)
        {
            ReleaseAsyncData(&timer);
            timer_hash_map_.erase(timer_key);
            return -1;
        }

        struct event* timer_event = event_new(timer_axis_ctx_.thread_ev_base, -1, EV_PERSIST,
                                              TimerAxis::OnTimer, callback_arg);
        if (NULL == timer_event)
        {
            const int err = errno;
            SET_LAST_ERR_MSG(&last_err_msg_, "failed to create timer event, errno: " << err
                             << ", err msg: " << strerror(err));

            callback_arg->Release();
            ReleaseAsyncData(&timer);
            timer_hash_map_.erase(timer_key);
            return -1;
        }

        if (event_add(timer_event, &interval) != 0)
        {
            const int err = errno;
            SET_LAST_ERR_MSG(&last_err_msg_, "failed to add timer event, errno: " << err
                             << ", err msg: " << strerror(err));

            event_free(timer_event);
            callback_arg->Release();
            ReleaseAsyncData(&timer);
            timer_hash_map_.erase(timer_key);
            return -1;
        }

        timer.timer_key = timer_key;
        timer.event = timer_event;
    }

    return 0;
}

void TimerAxis::KillTimer(TimerSinkInterface* sink, TimerID timer_id)
{
    if (NULL == sink)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "invalid param");
        return;
    }

    TimerKey timer_key;
    timer_key.sink = sink;
    timer_key.timer_id = timer_id;

    TimerHashMap::iterator it = timer_hash_map_.find(timer_key);
    if (it != timer_hash_map_.end())
    {
        Timer& timer = it->second;
        event_del(timer.event);
        timer.removed = true;
    }
}

int TimerAxis::FillAsyncData(Timer* timer, const void* async_data, size_t async_data_len)
{
    if (NULL == async_data || async_data_len < 1)
    {
        // 没有异步数据需要暂存
        return 0;
    }

    ReleaseAsyncData(timer);

    timer->async_data = new char[async_data_len + 1];
    if (NULL == timer->async_data)
    {
        const int err = errno;
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to alloc memory, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    memcpy(timer->async_data, async_data, async_data_len);
    timer->async_data_len = async_data_len;

    return 0;
}

void TimerAxis::ReleaseAsyncData(Timer* timer)
{
    if (timer->async_data != NULL)
    {
        delete[] timer->async_data;
        timer->async_data = NULL;
        timer->async_data_len = 0;
    }
}

void TimerAxis::RemoveTimer(Timer* timer)
{
    ReleaseAsyncData(timer);

    if (timer->event != NULL)
    {
        event_free(timer->event);
        timer->event = NULL;
    }

    timer_hash_map_.erase(timer->timer_key);
}
}
