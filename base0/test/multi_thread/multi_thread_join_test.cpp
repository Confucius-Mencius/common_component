#include <pthread.h>
#include <unistd.h>
#include <stdio.h>

//     pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);

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
    pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_JOINABLE);

    pthread_t tid;
    pthread_create(&tid, &tattr, thread_routine, NULL);

    pthread_attr_destroy(&tattr);

    pthread_cancel(tid);
    pthread_join(tid, NULL);

    return 0;
}
