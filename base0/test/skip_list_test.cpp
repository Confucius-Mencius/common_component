#include "skip_list_test.h"
#include "skip_list.h"
#include <sys/time.h>

static int64_t current_ms()
{
    struct timeval tv;
    ::gettimeofday(&tv, NULL);
    return int64_t(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
}

static void test(int num)
{
    int64_t t1 = current_ms();

    SkipList* sl = new SkipList;
    for (int i = 0; i < num; ++i)
    {
        if (sl->Insert(i) != 0)
        {
            std::cout << "skip_list Insert " << i << " failed." << std::endl;
        }
    }

    int64_t t2 = current_ms();
    std::cout << "skip_list Insert used " << t2 - t1 << " ms" << std::endl;

    int64_t t3 = current_ms();
    for (int i = 0; i < num; ++i)
    {
        if (!sl->Find(i))
        {
            std::cout << "skip_list not found " << i << std::endl;
        }
    }

    int64_t t4 = current_ms();
    std::cout << "skip_list Find used " << t4 - t3 << " ms" << std::endl;
    delete sl;

    std::set<int> si;
    int64_t t5 = current_ms();
    for (int i = 0; i < num; ++i)
    {
        si.insert(i);
    }

    int64_t t6 = current_ms();
    std::cout << "rb_tree Insert used " << t6 - t5 << " ms" << std::endl;

    int64_t t7 = current_ms();
    for (int i = 0; i < num; ++i)
    {
        if (si.find(i) == si.end())
        {
            std::cout << "rb_tree not found " << i << std::endl;
        }
    }

    int64_t t8 = current_ms();
    std::cout << "rb_tree Find used " << t8 - t7 << " ms" << std::endl;
}

SkipListTest::SkipListTest()
{

}

SkipListTest::~SkipListTest()
{

}

void SkipListTest::Test001()
{
    test(100);
}
