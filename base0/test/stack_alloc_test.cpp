#include "test_util.h"

// 堆、栈上的内存分配算法？

namespace stack_alloc_test
{
typedef std::pair<int, int> TP;

struct Big
{
    int a;
    int b;
    int c;
    int d;
    int e;
};

struct Little
{
    int x;
};

struct S
{
    void* p;
    int size;
};

void Func(S* s)
{
    Big* big = (Big*) s->p;

    printf("addr of big->a: %p\n", &(big->a));
    printf("addr of big->b: %p\n", &(big->b));
    printf("addr of big->c: %p\n", &(big->c));
    printf("addr of big->d: %p\n", &(big->d));
    printf("addr of big->e: %p\n", &(big->e));
}

int StackTest()
{
    printf("------ stack test ------\n");

    S s;
    printf("addr of s: %p\n", &s);

    Little sl;
    printf("addr of sl: %p\n", &sl);

//    EXPECT_GT(&s, &sl);

    s.p = &sl;
    s.size = sizeof(sl);

    printf("s.p: %p\n", s.p);

    memset(s.p, 0, sizeof(s.size));

    Func(&s);
    printf("s.p: %p\n", s.p);

    return 0;
}

int HeapTest()
{
    printf("------ heap test ------\n");

    // 堆上分配的地址谁大谁小，分配算法？
    Big* sb = new Big;
    printf("addr of sb: %p\n", sb);

    Little* sl = new Little;
    printf("addr of sl: %p\n", sl);

//    EXPECT_LT(sb, sl);

    S s;
    s.p = sb;
    s.size = sizeof(Big);
    Func(&s);

    s.p = sl;
    s.size = sizeof(Little);
    Func(&s);

    delete sb;
    delete sl;

    return 0;
}

void Test001()
{
    StackTest();
    HeapTest();

    TP x;
    x.first = 1;
    x.second = 2;
}

ADD_TEST(StackAllocTest, Test001);
} // namespace stack_alloc_test
