#include "timer_axis_test.h"
#include <event2/event.h>
#include "log_util.h"
#include "mem_util.h"
#include "mock_timer_sink.h"

using testing::NiceMock;
using testing::Return;

namespace timer_axis_test
{
static int n = 0;

class TimerSink : public TimerSinkInterface
{
public:
    TimerSink()
    {
        thread_ev_base_ = nullptr;
    }

    virtual ~TimerSink()
    {
    }

    void SetCtx(struct event_base* thread_ev_base)
    {
        thread_ev_base_ = thread_ev_base;
    }

    ///////////////////////// TimerSinkInterface /////////////////////////
    void OnTimer(TimerID timer_id, void* data, size_t len, int times) override
    {
        LOG_DEBUG("on timer, times: " << times);
        LOG_DEBUG("n: " << n);

        if (2 == n)
        {
            event_base_loopbreak(thread_ev_base_);
            return;
        }

        ++n;
    }

private:
    struct event_base* thread_ev_base_;
};

TimerAxisTest::TimerAxisTest() : loader_()
{
    thread_ev_base_ = nullptr;
    timer_axis_ = nullptr;
}

TimerAxisTest::~TimerAxisTest()
{
}

void TimerAxisTest::SetUp()
{
    thread_ev_base_ = event_base_new();
    if (nullptr == thread_ev_base_)
    {
        FAIL() << "failed to create event base";
    }

    if (loader_.Load("../libtimer_axis.so") != 0)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    timer_axis_ = static_cast<TimerAxisInterface*>(loader_.GetModuleInterface());
    if (nullptr == timer_axis_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    TimerAxisCtx timer_axis_ctx;
    timer_axis_ctx.thread_ev_base = thread_ev_base_;

    if (timer_axis_->Initialize(&timer_axis_ctx) != 0)
    {
        FAIL() << timer_axis_->GetLastErrMsg();
    }

    if (timer_axis_->Activate() != 0)
    {
        FAIL() << timer_axis_->GetLastErrMsg();
    }
}

void TimerAxisTest::TearDown()
{
    SAFE_DESTROY_MODULE(timer_axis_, loader_);

    if (thread_ev_base_ != nullptr)
    {
        event_base_free(thread_ev_base_);
        thread_ev_base_ = nullptr;
    }
}

void TimerAxisTest::Test001()
{
    timer_axis_test::n = 0;

    struct timeval interval;
    interval.tv_sec = 1;
    interval.tv_usec = 0;

    timer_axis_test::TimerSink timer_sink;
    timer_sink.SetCtx(thread_ev_base_);

    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink, 1));
    ASSERT_EQ(0, timer_axis_->SetTimer(&timer_sink, 1, interval, nullptr, 0));
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink, 1));

    timer_axis_->KillTimer(&timer_sink, 1);
    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink, 1));
    ASSERT_EQ(0, timer_axis_->SetTimer(&timer_sink, 1, interval, nullptr, 0));
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink, 1));

    event_base_dispatch(thread_ev_base_);
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink, 1));
}

/**
 * @brief 在回调过程中移除自己
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void TimerAxisTest::Test002()
{
    struct timeval interval;
    interval.tv_sec = 1;
    interval.tv_usec = 0;

    MockTimerSink1 timer_sink;
    timer_sink.Delegate(thread_ev_base_, timer_axis_);

    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink, 1));
    ASSERT_EQ(0, timer_axis_->SetTimer(&timer_sink, 1, interval, nullptr, 0));
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink, 1));

    EXPECT_CALL(timer_sink, OnTimer(1, nullptr, 0, 1)).Times(1);

    event_base_dispatch(thread_ev_base_);
    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink, 1));
}

/**
 * @brief 在回调过程中先移除自己，再添加自己
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void TimerAxisTest::Test003()
{
    struct timeval interval;
    interval.tv_sec = 1;
    interval.tv_usec = 0;

    MockTimerSink2 timer_sink;
    timer_sink.Delegate(thread_ev_base_, timer_axis_, interval);

    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink, 1));
    ASSERT_EQ(0, timer_axis_->SetTimer(&timer_sink, 1, interval, nullptr, 0));
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink, 1));

    EXPECT_CALL(timer_sink, OnTimer(1, nullptr, 0, _)).Times(3);

    event_base_dispatch(thread_ev_base_);
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink, 1));
}

/**
 * @brief 在回调过程中移除其它的定时器
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void TimerAxisTest::Test004()
{
    timer_axis_test::n = 0;

    struct timeval interval;
    interval.tv_sec = 1;
    interval.tv_usec = 0;

    timer_axis_test::TimerSink timer_sink;
    timer_sink.SetCtx(thread_ev_base_);

    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink, 1));
    ASSERT_EQ(0, timer_axis_->SetTimer(&timer_sink, 1, interval, nullptr, 0));
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink, 1));

    MockTimerSink3 timer_sink3;
    timer_sink3.Delegate(thread_ev_base_, timer_axis_, &timer_sink, 1);

    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink3, 1));
    ASSERT_EQ(0, timer_axis_->SetTimer(&timer_sink3, 1, interval, nullptr, 0));
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink3, 1));

    EXPECT_CALL(timer_sink3, OnTimer(1, nullptr, 0, _)).Times(3);

    event_base_dispatch(thread_ev_base_);
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink3, 1));
    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink, 1));
}

/**
 * @brief 在回调过程中添加其它的定时器
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void TimerAxisTest::Test005()
{
    struct timeval interval;
    interval.tv_sec = 1;
    interval.tv_usec = 0;

    MockTimerSink timer_sink;

    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink, 1));

    MockTimerSink4 timer_sink4;
    timer_sink4.Delegate(thread_ev_base_, timer_axis_, &timer_sink, 1, interval);

    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink4, 1));
    ASSERT_EQ(0, timer_axis_->SetTimer(&timer_sink4, 1, interval, nullptr, 0));
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink4, 1));

    EXPECT_CALL(timer_sink4, OnTimer(1, nullptr, 0, _)).Times(3);
    EXPECT_CALL(timer_sink, OnTimer(1, nullptr, 0, 1)).Times(1);

    event_base_dispatch(thread_ev_base_);
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink4, 1));
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink, 1));
}

/**
 * @brief util类测试
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void TimerAxisTest::Test006()
{
    struct timeval interval;
    interval.tv_sec = 1;
    interval.tv_usec = 0;

    const int ntimes = 10;

    MockTimerSink5 timer_sink;
    timer_sink.Delegate(thread_ev_base_, timer_axis_, ntimes);

    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink, 1));
    ASSERT_EQ(0, timer_axis_->SetTimer(&timer_sink, 1, interval, nullptr, 0, ntimes));
    ASSERT_TRUE(timer_axis_->TimerExist(&timer_sink, 1));

    EXPECT_CALL(timer_sink, OnTimer(1, nullptr, 0, _)).Times(ntimes);

    event_base_dispatch(thread_ev_base_);
    ASSERT_FALSE(timer_axis_->TimerExist(&timer_sink, 1));
}

ADD_TEST_F(TimerAxisTest, Test001);
ADD_TEST_F(TimerAxisTest, Test002);
ADD_TEST_F(TimerAxisTest, Test003);
ADD_TEST_F(TimerAxisTest, Test004);
ADD_TEST_F(TimerAxisTest, Test005);
ADD_TEST_F(TimerAxisTest, Test006);
} // namespace timer_axis_test
