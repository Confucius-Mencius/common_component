#include "thread_center_test_ex.h"
#include "log_util.h"

namespace thread_center_test
{
ThreadCenterTestEx::ThreadCenterTestEx() : loader_()
{
    thread_center_ = nullptr;
}

ThreadCenterTestEx::~ThreadCenterTestEx()
{
}

void ThreadCenterTestEx::SetUp()
{
    if (loader_.Load("../libthread_center.so") != 0)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    thread_center_ = static_cast<ThreadCenterInterface*>(loader_.GetModuleInterface());
    if (nullptr == thread_center_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    ASSERT_EQ(0, thread_center_->Initialize(nullptr));
    ASSERT_EQ(0, thread_center_->Activate());
}

void ThreadCenterTestEx::TearDown()
{
    SAFE_DESTROY_MODULE(thread_center_, loader_);
}

/**
 * @brief 新建一个只包含一个线程的线程组，作为源线程；再新建一个线程组，通知线程组中指定线程处理task，并接收响应，然后stop
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ThreadCenterTestEx::Test001()
{
    // 新建一个包含10个线程的线程组，与源线程通信
    ThreadGroupCtx thread_group_ctx;
    thread_group_ctx.common_component_dir = "..";
    thread_group_ctx.thread_name = "xx thread";
    thread_group_ctx.thread_count = 10;
    thread_group_ctx.thread_sink_creator = ThreadSinkEx::Create;

    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(&thread_group_ctx);
    ASSERT_TRUE(thread_group != nullptr);

    for (int i = 0; i < thread_group->GetThreadCount(); ++i)
    {
        ThreadInterface* thread = thread_group->GetThread(i);
        LOG_DEBUG("thread name: " << thread->GetThreadName() << ", idx: " << thread->GetThreadIdx());
    }

    // 新建源线程组，只包含一个线程
    ThreadGroupCtx source_thread_group_ctx;
    source_thread_group_ctx.common_component_dir = "..";
    source_thread_group_ctx.thread_name = "yy thread";
    source_thread_group_ctx.thread_count = 1;
    source_thread_group_ctx.thread_sink_creator = SourceThreadSink::Create;

    ThreadGroupInterface* source_thread_group = thread_center_->CreateThreadGroup(&source_thread_group_ctx);
    ASSERT_TRUE(source_thread_group != nullptr);

    SourceThreadSink* source_thread_sink = static_cast<SourceThreadSink*>(source_thread_group->GetThread(0)->GetThreadSink());
    source_thread_sink->SetReleatedThreadGroup(thread_group);

    // 线程组都active好了才可以start
    EXPECT_EQ(0, thread_group->Activate());
    EXPECT_EQ(0, source_thread_group->Activate());
    EXPECT_EQ(0, thread_group->Start());
    EXPECT_EQ(0, source_thread_group->Start());

    // 在源线程reload中push一个task给10个线程的线程组
    source_thread_group->NotifyReload();

    // stop
    thread_group->NotifyStop();
    source_thread_group->NotifyStop();

    if (thread_group->CanExit())
    {
        thread_group->NotifyExit();
    }

    if (source_thread_group->CanExit())
    {
        source_thread_group->NotifyExit();
    }

    thread_group->Join();
    source_thread_group->Join();

    SAFE_DESTROY(thread_group);
    SAFE_DESTROY(source_thread_group);
}

ADD_TEST_F(ThreadCenterTestEx, Test001);
}
