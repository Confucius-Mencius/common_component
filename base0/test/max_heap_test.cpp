#include "test_util.h"
#include "max_heap.h"

static void Test001()
{
    MaxHeap::Data array[10];
    int a[10] = {2, 6, 3, 8, 9, 7, 1, 4, 0, 5};

    for (int i = 0; i < 10; ++i)
    {
        array[i].key = a[i];
        array[i].value = NULL;
    }

    MaxHeap max_heap = MaxHeap(array, 10);
    max_heap.BuildHeap();

    for (int i = 0; i < 10; ++i)
    {
        std::cout << array[i].key << " ";
    }

    std::cout << std::endl;

    max_heap.Sort();

    for (int i = 0; i < 10; ++i)
    {
        std::cout << array[i].key << " ";
    }

    std::cout << std::endl;
}

ADD_TEST(MaxHeapTest, Test001);
