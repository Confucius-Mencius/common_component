#include <pthread.h>
#include <stdio.h>
#include <list>

pthread_mutex_t alarm_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t alarm_cond = PTHREAD_COND_INITIALIZER;

struct Msg
{
    int a;
};

typedef std::list<Msg> MsgList;
MsgList msg_list;

int loop_max = 100;

void* thread_routine(void* arg)
{
    int status;

    for (;;)
    {
        status = pthread_mutex_lock(&alarm_mutex);
        if (status != 0)
        {
        }

        while (msg_list.empty())
        {
            status = pthread_cond_wait(&alarm_cond, &alarm_mutex);
            if (status != 0)
            {
            }
        }

        // 取出一个消息
        Msg msg = msg_list.front();
        msg_list.pop_front();

        status = pthread_mutex_unlock(&alarm_mutex);
        if (status != 0)
        {
        }

        // 处理取出的这个消息
        printf("%d\n", msg.a);
        if (msg.a == loop_max)
        {
            break;
        }
    }

    printf("thread exit");
    return (void*) 0;
}

void PushMsg(const Msg& msg)
{
    int status;

    status = pthread_mutex_lock(&alarm_mutex);
    if (status != 0)
    {
    }

    // 向消息队列中添加消息
    msg_list.push_back(msg);

    status = pthread_mutex_unlock(&alarm_mutex);
    if (status != 0)
    {
    }

    status = pthread_cond_signal(&alarm_cond);
    if (status != 0)
    {
    }
}

int main()
{
    pthread_t pid;
    int status;

    status = pthread_create(&pid, NULL, thread_routine, NULL);
    if (status != 0)
    {
    }

    Msg msg;

    for (int i = 0; i < loop_max; ++i)
    {
        // 向消息队列中添加消息
        msg.a = i;
        PushMsg(msg);
    }

//    pthread_cancel(pid);

//    通知退出
    msg.a = loop_max;
    PushMsg(msg);

    pthread_join(pid, NULL);

    return 0;
}
