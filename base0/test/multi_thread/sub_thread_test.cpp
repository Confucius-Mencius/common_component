#include "sub_thread.h"
#include <stdio.h>

struct Msg
{
    int a;
};

class MySubThread : public SubThread<Msg>
{
public:
    MySubThread();
    virtual ~MySubThread();

    virtual int NotifyStop();
    virtual void HandleMsg(const Msg& msg);

private:

};

MySubThread::MySubThread()
{

}

MySubThread::~MySubThread()
{

}

int MySubThread::NotifyStop()
{
    Msg msg;
    msg.a = 100;
    PushBaskMsg(msg);

    return 0;
}

void MySubThread::HandleMsg(const Msg& msg)
{
    printf("%d\n", msg.a);

    if (msg.a == 100)
    {
        exit_ = true;
    }
}

int main()
{
    MySubThread my_sub_thread;
    if (my_sub_thread.Create() != 0)
    {
        return -1;
    }

    Msg msg;

    for (int i = 0; i < 100; ++i)
    {
        msg.a = i;

        if (my_sub_thread.PushBaskMsg(msg) != 0)
        {
            return -1;
        }
    }

    my_sub_thread.NotifyStop();
    my_sub_thread.Join();

    return 0;
}
