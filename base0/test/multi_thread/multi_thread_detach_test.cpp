#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

//     pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

void* thread_routine(void*)
{
    while (true)
    {
        //sleep(1);
    }

    printf("xx");
    return (void*) 0;
}

int main()
{
    pthread_attr_t tattr;
    pthread_attr_init(&tattr);
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);

    pthread_t tid;
    pthread_create(&tid, &tattr, thread_routine, NULL);

    pthread_attr_destroy(&tattr);

    pthread_cancel(tid);
//    pthread_join(tid, NULL);

    // 这里主线程退出了，整个进程就退出了
    return 0;
}
