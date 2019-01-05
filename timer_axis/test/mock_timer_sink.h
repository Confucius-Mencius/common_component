#ifndef TIMER_AXIS_TEST_MOCK_TIMER_SINK_H_
#define TIMER_AXIS_TEST_MOCK_TIMER_SINK_H_

#include <gmock/gmock.h>
#include "log_util.h"
#include "timer_axis_interface.h"

using ::testing::_;
using ::testing::Invoke;

namespace timer_axis_test
{
class MockTimerSink : public TimerSinkInterface
{
public:
    MOCK_METHOD4(OnTimer, void(TimerID timer_id, void* data, size_t len, int times));
};

// 在回调的时候移除自己
class TimerSink1 : public TimerSinkInterface
{
public:
    void SetCtx(struct event_base* thread_event_base, TimerAxisInterface* timer_axis, TimerSinkInterface* sink)
    {
        thread_event_base_ = thread_event_base;
        timer_axis_ = timer_axis;
        sink_ = sink;
    }

    virtual void OnTimer(TimerID timer_id, void* data, size_t len, int times)
    {
        (void) data;
        (void) len;

        LOG_DEBUG("on timer, times: " << times);
        timer_axis_->KillTimer(sink_, timer_id);
        event_base_loopbreak(thread_event_base_);
    }

private:
    struct event_base* thread_event_base_;
    TimerAxisInterface* timer_axis_;
    TimerSinkInterface* sink_;
};

class MockTimerSink1 : public TimerSinkInterface
{
public:
    MOCK_METHOD4(OnTimer, void(TimerID timer_id, void* data, size_t len, int times));

    void Delegate(struct event_base* thread_event_base, TimerAxisInterface* timer_axis)
    {
        timer_sink_.SetCtx(thread_event_base, timer_axis, this);
        ON_CALL(*this, OnTimer(_, _, _, _)).WillByDefault(Invoke(&timer_sink_, &TimerSink1::OnTimer));
    }

private:
    TimerSink1 timer_sink_;
};

// 在回调的时候先移除自己，然后再添加自己
class TimerSink2 : public TimerSinkInterface
{
public:
    void SetCtx(struct event_base* thread_event_base, TimerAxisInterface* timer_axis, TimerSinkInterface* sink,
                const struct timeval& interval)
    {
        thread_event_base_ = thread_event_base;
        timer_axis_ = timer_axis;
        sink_ = sink;
        interval_ = interval;
    }

    virtual void OnTimer(TimerID timer_id, void* data, size_t len, int times)
    {
        (void) data;
        (void) len;

        LOG_DEBUG("on timer, times: " << times);
        timer_axis_->KillTimer(sink_, timer_id);
        timer_axis_->SetTimer(sink_, timer_id, interval_, NULL, 0);

        static int n = 0;

        if (2 == n)
        {
            event_base_loopbreak(thread_event_base_);
        }

        ++n;
    }

private:
    struct event_base* thread_event_base_;
    TimerAxisInterface* timer_axis_;
    TimerSinkInterface* sink_;
    struct timeval interval_;
};

class MockTimerSink2 : public TimerSinkInterface
{
public:
    MOCK_METHOD4(OnTimer, void(TimerID timer_id, void* data, size_t len, int times));

    void Delegate(struct event_base* thread_event_base, TimerAxisInterface* timer_axis, const struct timeval& interval)
    {
        timer_sink_.SetCtx(thread_event_base, timer_axis, this, interval);
        ON_CALL(*this, OnTimer(_, _, _, _)).WillByDefault(Invoke(&timer_sink_, &TimerSink2::OnTimer));
    }

private:
    TimerSink2 timer_sink_;
};

// 在回调的时候移除其它的定时器
class TimerSink3 : public TimerSinkInterface
{
public:
    void SetCtx(struct event_base* thread_event_base, TimerAxisInterface* timer_axis, TimerSinkInterface* other_sink, TimerID other_id)
    {
        thread_event_base_ = thread_event_base;
        timer_axis_ = timer_axis;
        other_sink_ = other_sink;
        other_id_ = other_id;
    }

    virtual void OnTimer(TimerID timer_id, void* data, size_t len, int times)
    {
        (void) timer_id;
        (void) data;
        (void) len;

        LOG_DEBUG("on timer, times: " << times);
        timer_axis_->KillTimer(other_sink_, other_id_);

        static int n = 0;

        if (2 == n)
        {
            event_base_loopbreak(thread_event_base_);
        }

        ++n;
    }

private:
    struct event_base* thread_event_base_;
    TimerAxisInterface* timer_axis_;
    TimerSinkInterface* other_sink_;
    TimerID other_id_;
};

class MockTimerSink3 : public TimerSinkInterface
{
public:
    MOCK_METHOD4(OnTimer, void(TimerID timer_id, void* data, size_t len, int times));

    void Delegate(struct event_base* thread_event_base, TimerAxisInterface* timer_axis, TimerSinkInterface* other_sink, TimerID other_id)
    {
        timer_sink.SetCtx(thread_event_base, timer_axis, other_sink, other_id);
        ON_CALL(*this, OnTimer(_, _, _, _)).WillByDefault(Invoke(&timer_sink, &TimerSink3::OnTimer));
    }

private:
    TimerSink3 timer_sink;
};

// 在回调的时候添加其它的定时器
class TimerSink4 : public TimerSinkInterface
{
public:
    void SetCtx(struct event_base* thread_event_base, TimerAxisInterface* timer_axis, TimerSinkInterface* other_sink,
                TimerID other_id, const struct timeval& other_interval)
    {
        thread_event_base_ = thread_event_base;
        timer_axis_ = timer_axis;
        other_sink_ = other_sink;
        other_id_ = other_id;
        other_interval_ = other_interval;
    }

    virtual void OnTimer(TimerID timer_id, void* data, size_t len, int times)
    {
        (void) timer_id;
        (void) data;
        (void) len;

        LOG_DEBUG("on timer, times: " << times);
        timer_axis_->SetTimer(other_sink_, other_id_, other_interval_, NULL, 0);

        static int n = 0;

        if (2 == n)
        {
            event_base_loopbreak(thread_event_base_);
        }

        ++n;
    }

private:
    struct event_base* thread_event_base_;
    TimerAxisInterface* timer_axis_;
    TimerSinkInterface* other_sink_;
    TimerID other_id_;
    struct timeval other_interval_;
};

class MockTimerSink4 : public TimerSinkInterface
{
public:
    MOCK_METHOD4(OnTimer, void(TimerID timer_id, void* data, size_t len, int times));

    void Delegate(struct event_base* thread_event_base, TimerAxisInterface* timer_axis, TimerSinkInterface* other_sink,
                  TimerID other_id, const struct timeval& other_interval)
    {
        timer_sink_.SetCtx(thread_event_base, timer_axis, other_sink, other_id, other_interval);
        ON_CALL(*this, OnTimer(_, _, _, _)).WillByDefault(Invoke(&timer_sink_, &TimerSink4::OnTimer));
    }

private:
    TimerSink4 timer_sink_;
};

// 在回调的时候打印定时器调用的次数
class TimerSink5 : public TimerSinkInterface
{
public:
    void SetCtx(struct event_base* thread_event_base, TimerAxisInterface* timer_axis, TimerSinkInterface* sink, int ntimes)
    {
        thread_event_base_ = thread_event_base;
        timer_axis_ = timer_axis;
        sink_ = sink;
        ntimes_ = ntimes;
    }

    virtual void OnTimer(TimerID timer_id, void* data, size_t len, int times)
    {
        (void) timer_id;
        (void) data;
        (void) len;

        LOG_DEBUG("on timer, times: " << times);

        if (times >= ntimes_)
        {
            event_base_loopbreak(thread_event_base_);
        }
    }

private:
    struct event_base* thread_event_base_;
    TimerAxisInterface* timer_axis_;
    TimerSinkInterface* sink_;
    int ntimes_;
};

class MockTimerSink5 : public TimerSinkInterface
{
public:
    MOCK_METHOD4(OnTimer, void(TimerID timer_id, void* data, size_t len, int ntimes));

    void Delegate(struct event_base* thread_event_base, TimerAxisInterface* timer_axis, int ntimes)
    {
        timer_sink_.SetCtx(thread_event_base, timer_axis, this, ntimes);
        ON_CALL(*this, OnTimer(_, _, _, _)).WillByDefault(Invoke(&timer_sink_, &TimerSink5::OnTimer));
    }

private:
    TimerSink5 timer_sink_;
};
}

#endif // TIMER_AXIS_TEST_MOCK_TIMER_SINK_H_
