#include "weak_ptr_test.h"
#include <memory>

//weak_ptr不是一个独立的指针，而是shared_ptr的一个附加物。

//weak_ptr是为了配合shared_ptr而引入的一种智能指针，因为它不具有普通指针的行为，没有重载operator*和->,
//它的最大作用在于协助shared_ptr工作，像旁观者那样观测资源的使用情况。
//weak_ptr可以从一个shared_ptr或者另一个weak_ptr对象构造，获得资源的观测权。
//但weak_ptr没有共享资源，它的构造不会引起指针引用计数的增加，析构也不会引起引用计数的减少。
//使用weak_ptr的成员函数use_count()可以观测资源的引用计数，另一个成员函数expired()的功能等价于use_count()==0，但更快，
//表示被观测的资源(也就是shared_ptr的管理的资源)已经不复存在。
//weak_ptr可以使用一个非常重要的成员函数lock()从被观测的shared_ptr获得一个可用的shared_ptr对象， 从而操作资源。
//但当expired()==true的时候，lock()函数将返回一个存储空指针的shared_ptr。

//weak_ptr<T> w...............声明一个weak_ptr指针w，指向T型对象
//weak_ptr<T> w(p)............声明并且初始化，使其值为p
//w = p.......................赋值。p一般是shared_ptr或者weak_ptr
//w.reset()...................使w指向null
//w.use_count()...............所有与weak_ptr指向同一对象的shared_ptr的数量
//w.expired().................weak_ptr的生命期，指向对象还存在就返回true
//w.lock()....................生命期未结束，则返回一个shared_ptr，指向同样的对象。否则返回一个指向null的shared_ptr。
// 在lock()成功时会延长shared_ptr对象的生命周期,因为它递增了一个引用计数.


// weak_ptr没有release方法
// weak_ptr的reset(): reset后，weak_ptr变为expired，不可用了，但是不影响相应的shared_ptr
// 不能将weak_ptr置为null
// 当我们reset了shared_ptr之后， weak_ptr会自动“到期”（expired），不会产生悬空指针。

//weak_ptr使用场景：
//1、多线程空悬指针问题:
//    有两个指针p1和p2，指向堆上的同一个对象Object，p1和p2位于不同的线程中。假设线程A通过p1指针将对象销毁了（尽管把p1置为了NULL），那p2就成了空悬指针。这是一种典型的C/C++内存错误。

//使用weak_ptr能够帮我们轻松解决上述的空悬指针问题。

//weak_ptr不控制对象的生命期，但是它知道对象是否还活着。如果对象还活着，那么它可以提升为有效的shared_ptr（提升操作通过lock()函数获取所管理对象的强引用指针）；如果对象已经死了，提升会失败，返回一个空的shared_ptr。
//2、循环引用问题


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

//    当shared_ptr spw指向的对象的引用计数为1时，将spw赋值为null，spw所管理的资源会被析构，那么wpw也就成为了悬空指针，也称为“过期”（expired），
//    可以通过expired()判断weak_ptr是否过期。
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
        wpw.reset();
//        wpw = nullptr; // error
    }
}

void WeakPtrTest::Test002()
{
    //不使用智能指针常常见到的场景：
    int* ptr = new int(10);
    int* ref = ptr;
    (void) ref;
    delete ptr;    //此时ref就变成了悬空指针

    //有了智能指针之后，我们可以这样用：
    std::shared_ptr<int> sptr(new int(10));  //shared初始化
    std::weak_ptr<int> wptr1 = sptr;   //与shared_ptr指向同一对象
    sptr.reset(new int(20));  //shared_ptr指向新的对象。当我们reset了shared_ptr之后， weak_ptr会自动“到期”（expired）
    std::weak_ptr<int> wptr2 = sptr;  //用新的weak_ptr

    if (auto tmp = wptr1.lock()) //lock()返回shared_ptr，非空则输出
    {
        std::cout << *tmp << std::endl;
    }
    else
    {
        std::cout << "expired!" << std::endl;
    }

    if (auto tmp = wptr2.lock()) //lock()是weak_ptr成员函数
    {
        std::cout << *tmp << std::endl;
    }
    else
    {
        std::cout << "expired!" << std::endl;
    }
}

void WeakPtrTest::Test003()
{
    std::shared_ptr<weak_ptr_test::Person> sp (new weak_ptr_test::Person(10));
    std::weak_ptr<weak_ptr_test::Person> wp(sp);

    std::cout << "1. wp " << (wp.expired() ? "is" : "is not") << " expired" << std::endl;
    wp.reset(); // The object becomes empty, as if default constructed.
    std::cout << "2. wp " << (wp.expired() ? "is" : "is not") << " expired" << std::endl;

    std::weak_ptr<weak_ptr_test::Person> wp2(sp);
    std::cout << wp2.expired() << std::endl;
}

ADD_TEST_F(WeakPtrTest, Test001);
ADD_TEST_F(WeakPtrTest, Test002);
ADD_TEST_F(WeakPtrTest, Test003);

//https://blog.csdn.net/jxianxu/article/details/72859301
//https://blog.csdn.net/jxianxu/article/details/72859800
