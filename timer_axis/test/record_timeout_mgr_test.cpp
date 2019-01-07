#include "record_timeout_mgr_test.h"
#include <event2/event.h>
#include "log_util.h"
#include "mem_util.h"

namespace timer_axis_test
{
MyRecordTimeoutMgr::MyRecordTimeoutMgr()
{
    i_ = 0;
}

MyRecordTimeoutMgr::~MyRecordTimeoutMgr()
{
}

void MyRecordTimeoutMgr::OnTimeout(const Key& k, const Value& v, int timeout_sec)
{
    (void) k;
    (void) v;
    (void) timeout_sec;

#if !defined(NDEBUG)
    LOG_DEBUG("OnTimeout, key: " << k);

    if (0 == GetRecordCount())
    {
        event_base_loopbreak(thread_event_base_);
    }
#else
    event_base_loopbreak(thread_event_base_);
#endif
}

RecordTimeoutMgrTest::RecordTimeoutMgrTest()
{
}

RecordTimeoutMgrTest::~RecordTimeoutMgrTest()
{
}

void RecordTimeoutMgrTest::SetUp()
{
    thread_event_base_ = event_base_new();
    if (NULL == thread_event_base_)
    {
        FAIL() << "failed to create event base";
    }

    if (loader_.Load("../libtimer_axis.so") != 0)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    timer_axis_ = (TimerAxisInterface*) loader_.GetModuleInterface();
    if (NULL == timer_axis_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    TimerAxisCtx timer_axis_ctx;
    timer_axis_ctx.thread_ev_base = thread_event_base_;

    if (timer_axis_->Initialize(&timer_axis_ctx) != 0)
    {
        FAIL() << timer_axis_->GetLastErrMsg();
    }

    if (timer_axis_->Activate() != 0)
    {
        FAIL() << timer_axis_->GetLastErrMsg();
    }

    struct timeval expire_check_interval;
    expire_check_interval.tv_sec = 1;
    expire_check_interval.tv_usec = 0;

    ASSERT_EQ(0, record_timeout_mgr_.Initialize(timer_axis_, expire_check_interval));
    ASSERT_EQ(0, record_timeout_mgr_.Activate());
}

void RecordTimeoutMgrTest::TearDown()
{
    record_timeout_mgr_.Freeze();
    record_timeout_mgr_.Finalize();

    SAFE_DESTROY_MODULE(timer_axis_, loader_);

    if (thread_event_base_ != NULL)
    {
        event_base_free(thread_event_base_);
        thread_event_base_ = NULL;
    }
}

void RecordTimeoutMgrTest::Test001()
{
#if !defined(NDEBUG)
    timer_axis_test::MyRecordTimeoutMgr record_timeout_mgr;
    EXPECT_EQ(0, record_timeout_mgr.GetRecordCount());

    timer_axis_test::Key k1;
    k1.s = "i love you.";

    timer_axis_test::Value v1;
    v1.i = 1;

    const int timeout_sec1 = 1;

    record_timeout_mgr.UpsertRecord(k1, v1, timeout_sec1);
    EXPECT_EQ(1, record_timeout_mgr.GetRecordCount());
    record_timeout_mgr.Display();

    int timeout_sec;
    EXPECT_EQ(0, record_timeout_mgr.GetRecord(v1, timeout_sec, k1));
    EXPECT_EQ(1, v1.i);
    EXPECT_EQ(timeout_sec1, timeout_sec);

    const int timeout_sec2 = 2;
    record_timeout_mgr.UpsertRecord(k1, v1, timeout_sec2);
    EXPECT_EQ(1, record_timeout_mgr.GetRecordCount());
    record_timeout_mgr.Display();

    const int timeout_sec3 = 3;
    timer_axis_test::Key k3;
    k3.s = "i love you, too.";
    timer_axis_test::Value v3;
    v3.i = 3;

    record_timeout_mgr.UpsertRecord(k3, v3, timeout_sec3);
    EXPECT_EQ(2, record_timeout_mgr.GetRecordCount());
    record_timeout_mgr.Display();

    record_timeout_mgr.RemoveRecord(k3);
    EXPECT_EQ(1, record_timeout_mgr.GetRecordCount());
    record_timeout_mgr.Display();

    record_timeout_mgr.RemoveRecord(k1);
    EXPECT_EQ(0, record_timeout_mgr.GetRecordCount());
    record_timeout_mgr.Display();
#endif
}

void RecordTimeoutMgrTest::Test002()
{
    record_timeout_mgr_.SetEventBase(thread_event_base_);

#if !defined(NDEBUG)
    EXPECT_EQ(0, record_timeout_mgr_.GetRecordCount());
#endif

    timer_axis_test::Key k1;
    k1.s = "i love you.";

    timer_axis_test::Value v1;
    v1.i = 1;

    const int timeout_sec1 = 1;

    record_timeout_mgr_.UpsertRecord(k1, v1, timeout_sec1);
#if !defined(NDEBUG)
    EXPECT_EQ(1, record_timeout_mgr_.GetRecordCount());
    record_timeout_mgr_.Display();
#endif

    int timeout_sec;
    EXPECT_EQ(0, record_timeout_mgr_.GetRecord(v1, timeout_sec, k1));
    EXPECT_EQ(1, v1.i);
    EXPECT_EQ(timeout_sec1, timeout_sec);

    const int timeout_sec2 = 2;
    record_timeout_mgr_.UpsertRecord(k1, v1, timeout_sec2);
#if !defined(NDEBUG)
    EXPECT_EQ(1, record_timeout_mgr_.GetRecordCount());
    record_timeout_mgr_.Display();
#endif

    const int timeout_sec3 = 3;
    timer_axis_test::Key k3;
    k3.s = "i love you, too.";
    timer_axis_test::Value v3;
    v3.i = 3;

    record_timeout_mgr_.UpsertRecord(k3, v3, timeout_sec3);
#if !defined(NDEBUG)
    EXPECT_EQ(2, record_timeout_mgr_.GetRecordCount());
    record_timeout_mgr_.Display();
#endif

    event_base_dispatch(thread_event_base_);
}

ADD_TEST_F(RecordTimeoutMgrTest, Test001);
ADD_TEST_F(RecordTimeoutMgrTest, Test002);
}
