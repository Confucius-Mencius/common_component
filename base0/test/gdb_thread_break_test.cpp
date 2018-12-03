#include "test_util.h"
#include "thread_lock.h"
#include "simple_log.h"

//先介绍一下GDB多线程调试的基本命令。
//
//info threads 显示当前可调试的所有线程，每个线程会有一个GDB为其分配的ID，后面操作线程的时候会用到这个ID。 前面有*的是当前调试的线程。
//
//thread ID 切换当前调试的线程为指定ID的线程。
//
//break thread_test.c:123 thread all 在所有线程中相应的行上设置断点
//
//    thread apply ID1 ID2 command 让一个或者多个线程执行GDB命令command。
//
//thread apply all command 让所有被调试线程执行GDB命令command。
//
//set scheduler-locking off|on|step 估计是实际使用过多线程调试的人都可以发现，在使用step或者continue命令调试当前被调试线程的时候，其他线程也是同时执行的，怎么只让被调试程序执行呢？通过这个命令就可以实现这个需求。off 不锁定任何线程，也就是所有线程都执行，这是默认值。 on 只有当前被调试程序会执行。 step 在单步的时候，除了next过一个函数的情况(熟悉情况的人可能知道，这其实是一个设置断点然后continue的行为)以外，只有当前线程会执行。


//多线程调试之痛
//    调试器（如VS2008和老版GDB）往往只支持all-stop模式，调试多线程程序时，如果某个线程断在一个断点上，你的调试器会让整个程序freeze，直到你continue这个线程，程序中的其他线程才会继续运行。这个限制使得被调试的程序不能够像真实环境中那样运行--当某个线程断在一个断点上，让其他线程并行运行。
//
//GDBv7.0引入的non-stop模式使得这个问题迎刃而解。在这个模式下，
//
//当某个或多个线程断在一个断点上，其他线程仍会并行运行
//    你可以选择某个被断的线程，并让它继续运行
//    让我们想象一下，有了这个功能后
//
//    当其他线程断在断点上时，程序里的定时器线程可以正常的运行了，从而避免不必要得超时
//    当其他线程断在断点上时，程序里的watchdog线程可以正常的运行了，从而避免嵌入式硬件以为系统崩溃而重启
//    可以控制多个线程运行的顺序，从而重现deadlock场景了。由于GDB可以用python脚本驱动调试，理论上可以对程序在不同的线程运行顺序下进行自动化测试。
//因此，non-stop模式理所当然成为多线程调试“必杀技”。这2009年下半年之后发布的Linux版本里都带有GDBv7.0之后的版本。很好奇，不知道VS2010里是不是也支持类似的调试模式了。
//


//break thread_test.c:123 thread all  在所有线程中相应的行上设置断点，一个线程断住时，其它线程都会暂停运行。s或c后，其它线程都会开始执行
//break thread_test.c:123 thread 5  只在ID为5的线程上设置断点，线程5断住时，其它线程都会暂停。s或c后，其它线程都会开始执行
//set scheduler-locking on/off  在使用step或者continue命令调试当前被调试线程的时候，其他线程也是同时执行的。off 不锁定任何线程，也就是所有线程都执行，这是默认值。 on 只有当前被调试的线程会执行。

//1.前提
//gdb -v 查看版本信息
//7.0以前不支持non-stop模式
//
//2.线程锁
//set scheduler-locking on/off 打开关闭
//7.0以前的gdb默认当程序继续运行的时候如果有断点，那么就把所有的线程都
//    停下来，直到你指定某个线程继续执行(thread thread_no apply continue).
//但是如果直接在当前线程执行continue的话，默认是会启动所有线程。
//所以如果有多个线程都是断在同一个断点(函数)上，就是说这个函数是多线程的，
//那么调试的时候就会出现异常情况。所以需要打开线程锁。
//只让当前的线程执行，这意味着其他所有线程都不能执行了！
//
//
//3. non-stop模式
//set target-async 1
//set pagination off
//set non-stop on
//以上3个设置。同2不一样，gdb启动了non-stop模式，其实就是说，除了断点有关的线程会被停下来，
//其他线程会不会停下来。在网络程序调试的时候比较有用！
//
//4.所以可以根据gdb的版本及你的实际代码情况选择到底是线程锁还是non-stop模式
//
//5. 屏蔽SIGPIPE
//    gdb中的套接字等关闭了，往里面写就会报错！
//handle SIGPIPE nostop noprint ignore

ThreadMutex g_thread_mutex;
ThreadRWLock g_thread_rwlock;

static void* ThreadProc1(void* arg)
{
//    sleep(60);

    char* msg = (char*) arg;
    AUTO_THREAD_MUTEX(g_thread_mutex);

    for (int i = 0; i < 1; i++)
    {
        LOG_CPP((int) msg[0]);
    }

    return NULL;
}

static void Test001()
{
    std::cout << EXPAND_MACRO(AUTO_THREAD_MUTEX(g_thread_mutex)) << std::endl;

    pthread_t tid[10];
    char msg[10][2];

    for (int i = 0; i < 10; ++i)
    {
        msg[i][0] = (char) i;
        pthread_create(&tid[i], NULL, &ThreadProc1, (void*) msg[i]);
    }

    for (int i = 0; i < 10; ++i)
    {
        pthread_join(tid[i], NULL);
    }
}

static void* ThreadProc2(void* arg)
{
//    sleep(60);

    char* msg = (char*) arg;
    AUTO_THREAD_WLOCK(g_thread_rwlock);

    for (int i = 0; i < 1; i++)
    {
        LOG_CPP((int) msg[0]);
    }

    return NULL;
}

static void Test002()
{
    std::cout << EXPAND_MACRO(AUTO_THREAD_WLOCK(g_thread_rwlock)) << std::endl;
    std::cout << EXPAND_MACRO(AUTO_THREAD_RLOCK(g_thread_rwlock)) << std::endl;

    pthread_t tid[10];
    char msg[10][2];

    for (int i = 0; i < 10; ++i)
    {
        msg[i][0] = (char) i;
        pthread_create(&tid[i], NULL, &ThreadProc2, (void*) msg[i]);
    }

    for (int i = 0; i < 10; ++i)
    {
        pthread_join(tid[i], NULL);
    }
}

ADD_TEST(TreadLockTest, Test001);
ADD_TEST(TreadLockTest, Test002);
