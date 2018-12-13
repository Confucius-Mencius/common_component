#include "thread_center_test.h"

namespace thread_center_test
{
ThreadCenterTest::ThreadCenterTest() : loader_()
{
    thread_center_ = NULL;
}

ThreadCenterTest::~ThreadCenterTest()
{
}

void ThreadCenterTest::SetUp()
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

void ThreadCenterTest::TearDown()
{
    SAFE_DESTROY_MODULE(thread_center_, loader_);
}

/**
 * @brief 新建一个线程组，通知线程组stop
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ThreadCenterTest::Test001()
{
    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(NULL);
    ASSERT_TRUE(thread_group != NULL);

    for (int i = 0; i < 10; ++i)
    {
        ThreadCtx thread_ctx;
        thread_ctx.common_component_dir = "..";
        thread_ctx.idx = i;
        thread_ctx.name = "xx thread";
        thread_ctx.sink = ThreadSink::Create();
        ASSERT_TRUE(thread_ctx.sink != NULL);

        ThreadInterface* thread = thread_group->CreateThread(&thread_ctx);
        if (NULL == thread)
        {
            FAIL();
        }
    }

    thread_group->Activate();
    thread_group->Start();

    thread_group->NotifyStop();

    if (thread_group->CanExit())
    {
        thread_group->NotifyExit();
        thread_group->Join();
    }

    SAFE_DESTROY(thread_group);
}

/**
 * @brief 新建一个线程组，通知线程组reload，然后stop
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ThreadCenterTest::Test002()
{
    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(NULL);
    ASSERT_TRUE(thread_group != NULL);

    for (int i = 0; i < 10; ++i)
    {
        ThreadCtx thread_ctx;
        thread_ctx.common_component_dir = "..";
        thread_ctx.idx = i;
        thread_ctx.name = "xx thread";
        thread_ctx.sink = ThreadSink::Create();
        ASSERT_TRUE(thread_ctx.sink != NULL);

        ThreadInterface* thread = thread_group->CreateThread(&thread_ctx);
        if (NULL == thread)
        {
            FAIL();
        }
    }

    thread_group->Activate();
    thread_group->Start();

    thread_group->NotifyReload();
    thread_group->NotifyStop();

    if (thread_group->CanExit())
    {
        thread_group->NotifyExit();
        thread_group->Join();
    }

    SAFE_DESTROY(thread_group);
}

/**
 * @brief 新建一个线程组，通知线程组中指定线程处理task，然后stop
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ThreadCenterTest::Test003()
{
    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(NULL);
    ASSERT_TRUE(thread_group != NULL);

    for (int i = 0; i < 10; ++i)
    {
        ThreadCtx thread_ctx;
        thread_ctx.common_component_dir = "..";
        thread_ctx.idx = i;
        thread_ctx.name = "xx thread";
        thread_ctx.sink = ThreadSink::Create();
        ASSERT_TRUE(thread_ctx.sink != NULL);

        ThreadInterface* thread = thread_group->CreateThread(&thread_ctx);
        if (NULL == thread)
        {
            FAIL();
        }
    }

    thread_group->Activate();
    thread_group->Start();

    Task* task = new Task();
    ASSERT_TRUE(task != NULL);

    thread_group->PushTaskToThread(task, 5);
    thread_group->NotifyStop();

    if (thread_group->CanExit())
    {
        thread_group->NotifyExit();
        thread_group->Join();
    }

    SAFE_DESTROY(thread_group);
}

/**
 * @brief 新建一个线程组，广播通知线程组中所有线程处理task，然后stop
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ThreadCenterTest::Test004()
{
    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(NULL);
    ASSERT_TRUE(thread_group != NULL);

    for (int i = 0; i < 10; ++i)
    {
        ThreadCtx thread_ctx;
        thread_ctx.common_component_dir = "..";
        thread_ctx.idx = i;
        thread_ctx.name = "xx thread";
        thread_ctx.sink = ThreadSink::Create();
        ASSERT_TRUE(thread_ctx.sink != NULL);

        ThreadInterface* thread = thread_group->CreateThread(&thread_ctx);
        if (NULL == thread)
        {
            FAIL();
        }
    }

    thread_group->Activate();
    thread_group->Start();

    Task* task = new Task();
    ASSERT_TRUE(task != NULL);

    thread_group->PushTaskToThread(task, -1);
    thread_group->NotifyStop();

    if (thread_group->CanExit())
    {
        thread_group->NotifyExit();
        thread_group->Join();
    }

    SAFE_DESTROY(thread_group);
}

ADD_TEST_F(ThreadCenterTest, Test001);
ADD_TEST_F(ThreadCenterTest, Test002);
ADD_TEST_F(ThreadCenterTest, Test003);
ADD_TEST_F(ThreadCenterTest, Test004);
}
