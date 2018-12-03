#include "weak_ptr_test.h"
#include <memory>

//weak_ptr不是一个独立的指针，而是shared_ptr的一个附加物。

//weak_ptr是为了配合shared_ptr而引入的一种智能指针，因为它不具有普通指针的行为，没有重载operator*和->,它的最大作用在于协助shared_ptr工作，像旁观者那样观测资源的使用情况。
//weak_ptr可以从一个shared_ptr或者另一个weak_ptr对象构造，获得资源的观测权。但weak_ptr没有共享资源，它的构造不会引起指针引用计数的增加，析构也不会引起引用计数的减少。
//使用weak_ptr的成员函数use_count()可以观测资源的引用计数，另一个成员函数expired()的功能等价于use_count()==0,但更快，表示被观测的资源(也就是shared_ptr的管理的资源)已经不复存在。
//weak_ptr可以使用一个非常重要的成员函数lock()从被观测的shared_ptr获得一个可用的shared_ptr对象， 从而操作资源。但当expired()==true的时候，lock()函数将返回一个存储空指针的shared_ptr。

namespace weak_ptr_test
{
class Person
{
public:
    Person(int v)
    {
        value_ = v;
        std::cout << "Cons" << value_ << std::endl;
    }

    ~Person()
    {
        std::cout << "Des" << value_ << std::endl;
    }

private:
    int value_;
};
} // namespace weak_ptr_test

WeakPtrTest::WeakPtrTest()
{

}

WeakPtrTest::~WeakPtrTest()
{

}

void WeakPtrTest::Test001()
{
    {
        std::shared_ptr<int> sh_ptr = std::make_shared<int>(10);
        std::cout << sh_ptr.use_count() << std::endl;

        std::weak_ptr<int> wp(sh_ptr); // weak_ptr可以从一个shared_ptr或者另一个weak_ptr对象构造
        std::cout << wp.use_count() << std::endl;

        if (!wp.expired())
        {
            std::shared_ptr<int> sh_ptr2 = wp.lock(); //get another shared_ptr  // 如果wp过期，则返回null
            if (sh_ptr2 != nullptr)
            {
                *sh_ptr = 100;
            }

            std::cout << wp.use_count() << std::endl;
        }

        {
            std::weak_ptr<int> wp(sh_ptr); // 这个wp析构并不会引起对象引用计数的变化
            std::cout << wp.use_count() << std::endl;
        }

        std::cout << wp.use_count() << std::endl;
    }
    //delete memory

    auto spw = std::make_shared<weak_ptr_test::Person>(1);
    std::weak_ptr<weak_ptr_test::Person> wpw(spw);

    auto spw2 = spw;

//    当shared_ptr spw指向的对象的引用计数为1时，将spw赋值为null，spw所管理的资源会被析构，那么wpw也就成为了悬空指针，也称为“过期”（expired），可以通过expired()判断weak_ptr是否过期。
//    当其指向的对象的引用计数>1，也就是加上上面的语句后，将spw赋值为null，并不会析构对象。
    spw = nullptr;
    if (!wpw.expired())
    {
        std::cout << "not expired" << std::endl;
    }
    else
    {
        std::cout << "expired" << std::endl;
    }

    wpw.reset(); //   The object becomes empty, as if default constructed. reset后expired为true

    // weak_ptr没有release方法

    {
        // weak_ptr不能置为null
        auto spw = std::make_shared<weak_ptr_test::Person>(1);
        std::weak_ptr<weak_ptr_test::Person> wpw(spw);

//        wpw = nullptr; // error
    }
}

ADD_TEST_F(WeakPtrTest, Test001);

//https://blog.csdn.net/jxianxu/article/details/72859301
//https://blog.csdn.net/jxianxu/article/details/72859800