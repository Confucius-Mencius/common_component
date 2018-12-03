#include "thread_center_test_ex.h"

namespace thread_center_test
{
ThreadCenterTestEx::ThreadCenterTestEx() : thread_center_loader_()
{
    thread_center_ = NULL;
}

ThreadCenterTestEx::~ThreadCenterTestEx()
{
}

void ThreadCenterTestEx::SetUp()
{
    if (thread_center_loader_.Load("../libthread_center.so") != 0)
    {
        FAIL() << thread_center_loader_.GetLastErrMsg();
    }

    thread_center_ = (ThreadCenterInterface*) thread_center_loader_.GetModuleInterface();
    if (NULL == thread_center_)
    {
        FAIL() << thread_center_loader_.GetLastErrMsg();
    }

    ASSERT_EQ(0, thread_center_->Initialize(NULL));
    ASSERT_EQ(0, thread_center_->Activate());
}

void ThreadCenterTestEx::TearDown()
{
    if (thread_center_ != NULL)
    {
        thread_center_->Freeze();
        thread_center_->Finalize();
        thread_center_->Release();
        thread_center_ = NULL;
    }

    thread_center_loader_.Unload();
}

/**
 * @brief 新建一个只包含一个线程的线程组，作为upstream线程；再新建一个线程组，通知线程组中指定线程处理task，并接收响应，然后stop
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ThreadCenterTestEx::Test001()
{
    // 新建一个包含10个线程的线程组，与upstream线程通信
    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup();
    ASSERT_TRUE(thread_group != NULL);

    for (int i = 0; i < 10; ++i)
    {
        ThreadCtx thread_ctx;
        thread_ctx.common_component_dir = "..";
        thread_ctx.idx = i;
        thread_ctx.name = "xx thread";
        thread_ctx.sink = new ThreadSinkEx();
        ASSERT_TRUE(thread_ctx.sink != NULL);

        ThreadInterface* thread = thread_group->CreateThread(&thread_ctx);
        if (NULL == thread)
        {
            FAIL();
        }
    }

    for (int i = 0; i < thread_group->GetThreadCount(); ++i)
    {
        ThreadInterface* thread = thread_group->GetThread(i);
        std::cout << thread->GetThreadName() << ", " << thread->GetThreadIdx() << ", " << thread->GetThreadID()
            << ", " << thread->GetThreadEvBase() << ", " << thread->GetTimerAxis() << ", "
            << thread->GetConnCenterMgr() << ", " << thread->GetClientCenterMgr() << std::endl;
    }

    // 新建upstream线程组，只包含一个线程
    ThreadGroupInterface* upstream_thread_group = thread_center_->CreateThreadGroup();
    ASSERT_TRUE(upstream_thread_group != NULL);

    for (int i = 0; i < 1; ++i)
    {
        ThreadCtx thread_ctx;
        thread_ctx.common_component_dir = "..";
        thread_ctx.idx = i;
        thread_ctx.name = "global thread";
        thread_ctx.sink = UpstreamThreadSink::Create();
        ASSERT_TRUE(thread_ctx.sink != NULL);

        UpstreamThreadSinkCtx sink_ctx;
        sink_ctx.thread_group = thread_group;
        ((UpstreamThreadSink*) thread_ctx.sink)->SetSinkCtx(sink_ctx);

        ThreadInterface* thread = upstream_thread_group->CreateThread(&thread_ctx);
        if (NULL == thread)
        {
            FAIL();
        }
    }

    // 在upstream线程reload中push一个task给10个线程的线程组
    upstream_thread_group->NotifyReload();

    // stop
    thread_group->NotifyStop();

    if (thread_group->CanExit())
    {
        thread_group->NotifyExit();
    }

    thread_group->Freeze();
    thread_group->Finalize();
    thread_group->Release();

    upstream_thread_group->NotifyStop();

    if (upstream_thread_group->CanExit())
    {
        upstream_thread_group->NotifyExit();
    }

    upstream_thread_group->Freeze();
    upstream_thread_group->Finalize();
    upstream_thread_group->Release();
}

void ThreadCenterTestEx::Test002()
{

}

void ThreadCenterTestEx::Test003()
{

}

void ThreadCenterTestEx::Test004()
{

}

ADD_TEST_F(ThreadCenterTestEx, Test001);
ADD_TEST_F(ThreadCenterTestEx, Test002);
ADD_TEST_F(ThreadCenterTestEx, Test003);
ADD_TEST_F(ThreadCenterTestEx, Test004);
}
