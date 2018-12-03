#ifndef TIMER_AXIS_SRC_TIMER_AXIS_H_
#define TIMER_AXIS_SRC_TIMER_AXIS_H_

#include <set>
#include <event2/event.h>
#include "hash_container.h"
#include "last_err_msg.h"
#include "timer_axis_interface.h"

namespace timer_axis
{
struct TimerKey
{
    TimerSinkInterface* sink;
    TimerID timer_id;

    TimerKey()
    {
        sink = NULL;
        timer_id = 0;
    }

    bool operator==(const TimerKey& rhs) const
    {
        return (sink == rhs.sink) && (timer_id == rhs.timer_id);
    }

    bool operator<(const TimerKey& rhs) const
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

struct TimerKeyHash
{
    size_t operator()(const TimerKey& key) const
    {
        return (size_t) (key.sink);
    }
};

struct Timer
{
    TimerKey timer_key;
    struct event* event;
    char* async_data;
    size_t async_data_len;
    int total_times; // 定时器一共要运行多少次
    int times; // 已经运行了多少次
    bool removed;

    Timer() : timer_key()
    {
        event = NULL;
        async_data = NULL;
        async_data_len = 0;
        total_times = -1;
        times = 0;
        removed = false;
    }
};

class TimerAxis : public TimerAxisInterface
{
public:
    static void OnTimer(evutil_socket_t fd, short event, void* arg);

public:
    TimerAxis();
    virtual ~TimerAxis();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// TimerAxisInterface /////////////////////////
    bool TimerExist(TimerSinkInterface* sink, TimerID timer_id) override;
    int SetTimer(TimerSinkInterface* sink, TimerID timer_id, const struct timeval& interval,
                 const void* async_data = NULL, size_t async_data_len = 0, int total_times = -1) override;
    void KillTimer(TimerSinkInterface* sink, TimerID timer_id) override;

public:
    LastErrMsg& LastErrMsgInstance()
    {
        return last_err_msg_;
    }

private:
    int FillAsyncData(Timer* timer, const void* async_data, size_t async_data_len);
    void ReleaseAsyncData(Timer* timer);
    void RemoveTimer(Timer* timer);

private:
    LastErrMsg last_err_msg_;
    TimerAxisCtx timer_axis_ctx_;

    typedef __hash_map<TimerKey, Timer, TimerKeyHash> TimerHashMap;
    TimerHashMap timer_hash_map_;
};
}

#endif // TIMER_AXIS_SRC_TIMER_AXIS_H_
