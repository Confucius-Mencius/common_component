#include "thread_center_test.h"
#include <sys/sysinfo.h>
#include <thread>
#include "thread_task_count.h"

namespace thread_center_test
{
std::atomic<int> g_thread_task_count(0);

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
    ASSERT_EQ(0, g_thread_task_count);
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
    thread_group_ctx.thread_count = 50;
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
    thread_group_ctx.thread_count = 50;
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
    thread_group_ctx.thread_count = 50;
    thread_group_ctx.thread_sink_creator = ThreadSink::Create;

    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(&thread_group_ctx);
    ASSERT_TRUE(thread_group != NULL);

    thread_group->Start();

    ThreadTask* task = new ThreadTask();
    ASSERT_TRUE(task != NULL);
    g_thread_task_count++;

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
    thread_group_ctx.thread_count = 50;
    thread_group_ctx.thread_sink_creator = ThreadSink::Create;

    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(&thread_group_ctx);
    ASSERT_TRUE(thread_group != NULL);

    thread_group->Start();

    ThreadTask* task = new ThreadTask();
    ASSERT_TRUE(task != NULL);
    g_thread_task_count = thread_group_ctx.thread_count; // 广播接口推往每个线程的task都会new一个新的出来

    thread_group->PushTaskToThread(task, -1);
    thread_group->NotifyStop();

    if (thread_group->CanExit())
    {
        thread_group->NotifyExit();
        thread_group->Join();
    }

    SAFE_DESTROY(thread_group);
}

void ProducerThreadProcess(ThreadGroupInterface* thread_group, int thread_count)
{
    for (int i = 0; i < 10000000; ++i)
    {
        ThreadTask* task = new ThreadTask();
        ASSERT_TRUE(task != NULL);
        g_thread_task_count++;

        thread_group->PushTaskToThread(task, rand() % thread_count);
    }
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
    int n1 = get_nprocs_conf();
    (void) n1;

    int n2 = get_nprocs(); // 可用的cpu核數
    (void) n2;

    g_log_engine->SetLogLevel(log4cplus::ERROR_LOG_LEVEL);

    ThreadGroupCtx thread_group_ctx;
    thread_group_ctx.common_component_dir = "..";
    thread_group_ctx.thread_name = "xx thread";
    thread_group_ctx.thread_count = 2;
    thread_group_ctx.thread_sink_creator = ThreadSink::Create;

    ThreadGroupInterface* thread_group = thread_center_->CreateThreadGroup(&thread_group_ctx);
    ASSERT_TRUE(thread_group != NULL);

    thread_group->Start();

    std::vector<std::thread*> threads;

    for (int i = 0; i < 2; ++i)
    {
        threads.push_back(new std::thread(ProducerThreadProcess, thread_group, thread_group_ctx.thread_count));
    }

    for (int i = 0; i < (int) threads.size(); ++i)
    {
        threads[i]->join();
        delete threads[i];
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
