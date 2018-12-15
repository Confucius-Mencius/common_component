#include "thread_center_test.h"
#include "task_count.h"

namespace thread_center_test
{
std::atomic<int> g_task_count(0);

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
    ASSERT_EQ(0, g_task_count);
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
    ThreadGroupCtx thread_group_ctx;
    thread_group_ctx.common_component_dir = "..";
    thread_group_ctx.thread_name = "xx thread";
    thread_group_ctx.thread_count = 10;
    thread_group_ctx.thread_sink_creator = ThreadSink::Create;

    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(&thread_group_ctx);
    ASSERT_TRUE(thread_group != NULL);

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
    ThreadGroupCtx thread_group_ctx;
    thread_group_ctx.common_component_dir = "..";
    thread_group_ctx.thread_name = "xx thread";
    thread_group_ctx.thread_count = 10;
    thread_group_ctx.thread_sink_creator = ThreadSink::Create;

    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(&thread_group_ctx);
    ASSERT_TRUE(thread_group != NULL);

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
    ThreadGroupCtx thread_group_ctx;
    thread_group_ctx.common_component_dir = "..";
    thread_group_ctx.thread_name = "xx thread";
    thread_group_ctx.thread_count = 10;
    thread_group_ctx.thread_sink_creator = ThreadSink::Create;

    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(&thread_group_ctx);
    ASSERT_TRUE(thread_group != NULL);

    thread_group->Start();

    Task* task = new Task();
    ASSERT_TRUE(task != NULL);
    g_task_count++;

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
    ThreadGroupCtx thread_group_ctx;
    thread_group_ctx.common_component_dir = "..";
    thread_group_ctx.thread_name = "xx thread";
    thread_group_ctx.thread_count = 10;
    thread_group_ctx.thread_sink_creator = ThreadSink::Create;

    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(&thread_group_ctx);
    ASSERT_TRUE(thread_group != NULL);

    thread_group->Start();

    Task* task = new Task();
    ASSERT_TRUE(task != NULL);
    g_task_count = 10;

    thread_group->PushTaskToThread(task, -1);
    thread_group->NotifyStop();

    if (thread_group->CanExit())
    {
        thread_group->NotifyExit();
        thread_group->Join();
    }

    SAFE_DESTROY(thread_group);
}

/**
 * @brief 新建一个线程组，通知线程组中指定线程处理task，无限循环
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void ThreadCenterTest::Test005()
{
    ThreadGroupCtx thread_group_ctx;
    thread_group_ctx.common_component_dir = "..";
    thread_group_ctx.thread_name = "xx thread";
    thread_group_ctx.thread_count = 100;
    thread_group_ctx.thread_sink_creator = ThreadSink::Create;

    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(&thread_group_ctx);
    ASSERT_TRUE(thread_group != NULL);

    thread_group->Start();

    for (int i = 0; i < 10000000; ++i)
    {
        Task* task = new Task();
        ASSERT_TRUE(task != NULL);
        g_task_count++;

        thread_group->PushTaskToThread(task, rand() % thread_group_ctx.thread_count);
    }

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
ADD_TEST_F(ThreadCenterTest, Test005);
}
