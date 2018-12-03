#ifndef TIMER_AXIS_INC_TIMER_AXIS_UTIL_H_
#define TIMER_AXIS_INC_TIMER_AXIS_UTIL_H_

#include <set>
#include "timer_axis_interface.h"

/**
 * @brief 定时器助手类
 * @attention
 *  回调的一个潜在风险就是当回调发生的时候，回调对象已经被释放了，
 *  即开发者忘记了在销毁对象的时候删除相关的定时器，这里提供一个助手类，可以避免这种错误
 */
class TimerAxisUtil
{
public:
    TimerAxisUtil() : timer_set_()
    {
        timer_axis_ = NULL;
    }

    ~TimerAxisUtil()
    {
        if (NULL == timer_axis_)
        {
            return;
        }

        for (TimerSet::const_iterator it = timer_set_.begin(); it != timer_set_.end(); ++it)
        {
            const Timer& timer = *it;
            timer_axis_->KillTimer(timer.sink, timer.timer_id);
        }

        timer_set_.clear();
    }

    void SetTimerAxis(TimerAxisInterface* timer_axis)
    {
        timer_axis_ = timer_axis;
    }

    bool TimerExist(TimerSinkInterface* sink, TimerID timer_id)
    {
        if (NULL == timer_axis_)
        {
            return false;
        }

        return timer_axis_->TimerExist(sink, timer_id);
    }

    int SetTimer(TimerSinkInterface* sink, TimerID timer_id, struct timeval& interval, const void* async_data,
                 size_t async_data_len, int total_times = -1)
    {
        if (NULL == timer_axis_)
        {
            return -1;
        }

        if (timer_axis_->SetTimer(sink, timer_id, interval, async_data, async_data_len, total_times) != 0)
        {
            return -1;
        }

        Timer timer;
        timer.Create(sink, timer_id);

        if (!timer_set_.insert(timer).second)
        {
            timer_axis_->KillTimer(sink, timer_id);
            return -1;
        }

        return 0;
    }

    void KillTimer(TimerSinkInterface* sink, TimerID timer_id)
    {
        if (NULL == timer_axis_)
        {
            return;
        }

        Timer timer;
        timer.Create(sink, timer_id);

        TimerSet::const_iterator it = timer_set_.find(timer);
        if (it != timer_set_.end())
        {
            timer_axis_->KillTimer(timer.sink, timer.timer_id);
            timer_set_.erase(it);
        }
    }

private:
    struct Timer
    {
        TimerSinkInterface* sink;
        TimerID timer_id;

        void Create(TimerSinkInterface* sink, TimerID timer_id)
        {
            this->sink = sink;
            this->timer_id = timer_id;
        }

        bool operator<(const Timer& rhs) const
        {
            if (sink != rhs.sink)
            {
                return sink < rhs.sink;
            }
            else
            {
                return timer_id < rhs.timer_id;
            }
        }
    };

    typedef std::set<Timer> TimerSet;

    TimerAxisInterface* timer_axis_;
    TimerSet timer_set_;
};

#endif // TIMER_AXIS_INC_TIMER_AXIS_UTIL_H_
