#include <stdio.h>
#include <unistd.h>

// 性能分析报告

static void test()
{
    sleep(1);
}

static void f()
{
    for (int i = 0; i < 5; ++i)
    {
        test();
    }
}

int main(int argc, char* argv[])
{
    f();
    printf("process is over!\n");
    return 0;
}
