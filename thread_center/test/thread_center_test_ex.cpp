#include "thread_center_test_ex.h"
#include "log_util.h"

namespace thread_center_test
{
ThreadCenterTestEx::ThreadCenterTestEx() : loader_()
{
    thread_center_ = NULL;
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

    thread_center_ = (ThreadCenterInterface*) loader_.GetModuleInterface();
    if (NULL == thread_center_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    ASSERT_EQ(0, thread_center_->Initialize(NULL));
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
    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(NULL);
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

    thread_group->Activate();

    for (int i = 0; i < thread_group->GetThreadCount(); ++i)
    {
        ThreadInterface* thread = thread_group->GetThread(i);
        LOG_DEBUG(thread->GetThreadName() << ", " << thread->GetThreadIdx());
    }

    // 新建源线程组，只包含一个线程
    ThreadGroupInterface* source_thread_group = thread_center_->CreateThreadGroup(NULL);
    ASSERT_TRUE(source_thread_group != NULL);

    for (int i = 0; i < 1; ++i)
    {
        ThreadCtx thread_ctx;
        thread_ctx.common_component_dir = "..";
        thread_ctx.idx = i;
        thread_ctx.name = "yy thread";
        thread_ctx.sink = SourceThreadSink::Create();
        ASSERT_TRUE(thread_ctx.sink != NULL);

        SourceThreadSinkCtx sink_ctx;
        sink_ctx.thread_group = thread_group;
        ((SourceThreadSink*) thread_ctx.sink)->SetSinkCtx(sink_ctx);

        ThreadInterface* thread = source_thread_group->CreateThread(&thread_ctx);
        if (NULL == thread)
        {
            FAIL();
        }
    }

    source_thread_group->Activate();

    // 线程组都active好了才可以start
    thread_group->Start();
    source_thread_group->Start();

    // 在源线程reload中push一个task给10个线程的线程组
    source_thread_group->NotifyReload();

    // stop
    thread_group->NotifyStop();
    source_thread_group->NotifyStop();

    if (thread_group->CanExit())
    {
        thread_group->NotifyExit();
        thread_group->Join();
    }

    if (source_thread_group->CanExit())
    {
        source_thread_group->NotifyExit();
        source_thread_group->Join();
    }

    SAFE_DESTROY(thread_group);
    SAFE_DESTROY(source_thread_group);
}

ADD_TEST_F(ThreadCenterTestEx, Test001);
}
