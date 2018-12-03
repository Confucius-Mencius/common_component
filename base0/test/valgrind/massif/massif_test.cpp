#include <stdlib.h>
#include <stdio.h>

/**
 * 现实中总是遇到一些内存一直上涨，但是就是没有memory leaker的情况，
 * 其实这样的情况并不奇怪，非常容易发生，比如每次循环的时候你并没有把上次的内存释放，
 * 同时不断在后面分配更多的内存，这显然不会有memory leaker，倒是到了一定的程度，自然会发生bad alloc的问题。
 * 对于这样的问题massif就可以大显身手，比较一下两个内存的切片，增加的部分自然就发现了。
 */

static int* fa()
{
    int* p = (int*) malloc(10000);
    return p;
}

static void fb(int* p)
{
    delete p;
}

int main(int argc, char* argv[])
{
    printf("ok\n");

    printf("really ok?\n");

    int* vec[10000];

    for (int i = 0; i < 10000; ++i)
    {
        vec[i] = fa();
    }

    for (int i = 0; i < 10000; ++i)
    {
        fb(vec[i]);
    }

    return 0;
}
