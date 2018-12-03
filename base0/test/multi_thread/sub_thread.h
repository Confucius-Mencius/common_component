#ifndef SUB_THREAD_H_
#define SUB_THREAD_H_

#include <pthread.h>
#include <list>
#include <errno.h>

template<typename Msg>
class SubThread
{
public:
    static void* ThreadRoutine(void* arg);

public:
    SubThread();
    virtual ~SubThread();

    int Create();
    virtual int NotifyStop() = 0;
    int Join();
    int PushBaskMsg(const Msg& msg);
    virtual void HandleMsg(const Msg& msg) = 0;
    int GetLastErrno() const;

private:
    pthread_t tid_;
    pthread_mutex_t mutex_;
    pthread_cond_t cond_;

    typedef std::list<Msg> MsgList;
    MsgList msg_list_;
    int last_errno_;

protected:
    bool exit_;
};

template<typename Msg>
void* SubThread<Msg>::ThreadRoutine(void* arg)
{
    SubThread* thread = (SubThread*) arg;
    Msg msg;

    for (;;)
    {
        if (pthread_mutex_lock(&thread->mutex_) != 0)
        {
            return (void*) -1;
        }

        while (thread->msg_list_.empty())
        {
            if (pthread_cond_wait(&thread->cond_, &thread->mutex_) != 0)
            {
                return (void*) -2;
            }
        }

        // 取出消息队列头部的消息
        msg = thread->msg_list_.front();
        thread->msg_list_.pop_front();

        if (pthread_mutex_unlock(&thread->mutex_) != 0)
        {
            return (void*) -3;
        }

        // 处理取出的这个消息
        thread->HandleMsg(msg);

        if (thread->exit_)
        {
            break;
        }
    }

    return (void*) 0;
}

template<typename Msg>
SubThread<Msg>::SubThread() : msg_list_()
{
    tid_ = 0;
    pthread_mutex_init(&mutex_, NULL);
    pthread_cond_init(&cond_, NULL);
    last_errno_ = 0;
    exit_ = false;
}

template<typename Msg>
SubThread<Msg>::~SubThread()
{
    pthread_cond_destroy(&cond_);
    pthread_mutex_destroy(&mutex_);
}

template<typename Msg>
int SubThread<Msg>::Create()
{
    if (pthread_create(&tid_, NULL, ThreadRoutine, this) != 0)
    {
        last_errno_ = errno;
        return -1;
    }

    return 0;
}

template<typename Msg>
int SubThread<Msg>::Join()
{
    if (pthread_join(tid_, NULL) != 0)
    {
        last_errno_ = errno;
        return -1;
    }

    return 0;
}

template<typename Msg>
int SubThread<Msg>::PushBaskMsg(const Msg& msg)
{
    if (pthread_mutex_lock(&mutex_) != 0)
    {
        last_errno_ = errno;
        return -1;
    }

    // 向消息队列尾部追加消息
    msg_list_.push_back(msg);

    if (pthread_mutex_unlock(&mutex_) != 0)
    {
        last_errno_ = errno;
        return -2;
    }

    if (pthread_cond_signal(&cond_) != 0)
    {
        last_errno_ = errno;
        return -3;
    }

    return 0;
}

template<typename Msg>
int SubThread<Msg>::GetLastErrno() const
{
    return last_errno_;
}

#endif // SUB_THREAD_H_
