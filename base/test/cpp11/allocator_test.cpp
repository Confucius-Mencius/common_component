#include "allocator_test.h"
#include <memory>

//allocator类是一个模板类，定义在头文件memory中，用于内存的分配、释放、管理，它帮助我们将内存分配和对象构造分离开来。
//具体地说，allocator类将内存的分配和对象的构造解耦，分别用allocate和construct两个函数完成，
//同样将内存的释放和对象的析构销毁解耦，分别用deallocate和destroy函数完成。
//allocator类分配的内存是未构造的，为了使用已经分配好的内存，我们必须使用construct构造对象。如果使用未构造的内存，其行为是未定义的。
//只能对真正构造了的对象进行destroy操作，用户必须保证在调用deallocate函数回收内存前对这块内存上的每个元素调用destroy函数。

namespace cpp11_allocator_test
{
class Example
{
public:
    Example() : a(0)
    {
        std::cout << "example default constructor..." << std::endl;
    }

    Example(int x) : a(x)
    {
        std::cout << "example constructor..." << std::endl;
    }

    ~Example()
    {
        std::cout << "example destructor..." << std::endl;
    }

    int a;
};
}

AllocatorTest::AllocatorTest()
{

}

AllocatorTest::~AllocatorTest()
{

}

void AllocatorTest::Test001()
{
    std::cout << __cplusplus << std::endl;

    std::allocator<cpp11_allocator_test::Example> alloc;
    cpp11_allocator_test::Example* p = alloc.allocate(2);
    alloc.construct(p, cpp11_allocator_test::Example());
    std::cout << p->a << std::endl;
    alloc.destroy(p);

    alloc.construct(p + 1, cpp11_allocator_test::Example(3));
    std::cout << (p + 1)->a << std::endl;
    alloc.destroy(p + 1);

    alloc.deallocate(p, 2);
}

ADD_TEST_F(AllocatorTest, Test001);
