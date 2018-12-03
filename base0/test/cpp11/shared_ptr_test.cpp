#include "shared_ptr_test.h"
#include <memory>

//多个shared_ptr可以指向同一份资源。
// shared_ptr使用引用计数，每一个shared_ptr的拷贝都指向相同的内存。每使用他一次，内部的引用计数加1，每析构一次，内部的引用计数减1，减为0时，自动释放所指向的堆内存。
//shared_ptr内部的引用计数是安全的，但是对象的读取需要加锁。

//初始化
//　　可以通过构造函数、std::make_shared<T>辅助函数和reset方法来初始化shared_ptr：
//　  reset()包含两个操作。当智能指针中有值的时候，调用reset()会使引用计数减1.
//    当调用reset（new xxx())重新赋值时，智能指针首先是生成新对象，然后将旧对象的引用计数减1（当然，如果发现引用计数为0时，则析构旧对象），然后将新对象的指针交给智能指针保管。

//获取原始指针　　
//std::shared_ptr<int> p4(new int(5));
//int *pInt = p4.get();

//指定删除器
//　　智能指针可以指定删除器，当智能指针的引用计数为0时，自动调用指定的删除器来释放内存。std::shared_ptr可以指定删除器的一个原因是其默认删除器不支持数组对象


//shared_ptr有多种形式的构造函数，应用于各种可能的情形：
//
//▲ 无参的shared_ptr( )创建一个持有空指针的shared_ptr;
//
//▲ shared_ptr(Y *p)获得指向类型T的指针p的管理权，同时引用计数置为1。
//
//这个构造函数要求Y类型必须能够转换为T类型；
//
//▲ shared_ptr(shared_ptr const & r)从另外一个shared_ptr获得指针的管理
//
//    权，同时引用计数加1，结果是两个shared_ptr共享一个指针的管理权；
//
//▲ shared_ptr(std::auto_ptr<Y> & r)从一个auto_ptr获得指针的管理权，引用
//
//    计数置为1，同时auto_ptr自动失去管理权；
//
//▲ operator=赋值操作符可以从另外一个shared_ptr或auto_ptr获得指针的
//
//管理权，其行为同构造函数；
//
//▲ shared_ptr( Y *p, D d)行为类似shared_ptr（Y * p)，但使用参数d指定了
//
//    析构时的定制删除器，而不是简单的delete。
//
//
//shared_ptr还支持比较运算符，可以测试两个shared_ptr的相等或者不等，比较基于内部保存的指针，相当于a.get( ) == b.get( )。
//
//shared_ptr还可以使用operator<比较大小，同样基于内部保存的指针，但不提供除operator<以外的比较操作符，这使得shared_ptr可以被用于标准关联容器(set 和 map):
//
//shared_ptr还支持流输出操作符operator<<，输出内部的指针值，方便调试。
//
//shared_ptr有两个专门的函数检查引用计数。unique( )在shared_ptr是指针的唯一拥所有者时返回true。use_count( )返回当前指针的引用计数。


//使用shared_ptr需要注意的问题
//
//1. 不要用一个原始指针初始化多个shared_ptr，原因在于，会造成二次销毁，如下所示：
//int *p5 = new int;
//std::shared_ptr<int> p6(p5);
//std::shared_ptr<int> p7(p5);// logic error
//
//2. 不要在函数实参中创建shared_ptr。因为C++的函数参数的计算顺序在不同的编译器下是不同的。正确的做法是先创建好，然后再传入。
//function(shared_ptr<int>(new int), g());
//
//3. 避免循环引用。智能指针最大的一个陷阱是循环引用，循环引用会导致内存泄漏。解决方法是AStruct或BStruct改为weak_ptr。

//相比于普通指针，共享指针要占用多一倍的内存空间，其内部包含两个指针，一个指针指向它所管理的资源，第二个指针指向一个称为“Control Block”的控制块
//在控制块中有一个引用计数字段（Reference Count），用来记录指向Object的共享指针数量，当我们声明并初始化一个共享指针时，Reference Count的值为1，
//当有另一个共享指针指向该Object时（比如通过赋值或拷贝构造函数，将一个共享指针赋值给另一共享指针），Reference count 的值递增1。
//第二个字段：Weak Count，也是一个引用计数，它用来计数指向该Object的std::weak_ptr指针的数量
//资源释放器的地址存放于控制块（Control Block）中，因此增加用户自定义的资源释放器不会增加shared_ptr的大小，其大小仍然为两个指针的大小。
//还有Allocator的地址也在控制块中。


//当代码第一次为一个对象创建shared_ptr指针指向这个对象的时候，控制块就会被创建，或者说它应该被创建。但是问题在于，一处代码在创建shared_ptr来指向一个对象时，这处代码并不知道它创建的这个shared_ptr所要指向的对象是否已经有别的shared_ptr指向了，即这个对象是否已经有一个控制块与之对应了。为了解决这个问题，控制块的创建遵循以下原则：
//
//std::make_shared总是会创建一个控制块。这是因为当调用std::make_shared创建智能指针的时候，智能指针所要指向的这个对象是第一次被创建的。
//
//当通过一个普通指针作为参数构建一个智能指针的时候，会为这个指针所指向的对象创建一个控制块。
//
//在这种情况下，如果用户想要为一个已经有控制块的对象创建一个shared_ptr时，需要用shared_ptr或者weak_ptr作为共享指针构造函数的参数，而不能用普通指针作为参数。
//
//注意：使用普通指针作为参数构造一个shared_ptr有可能会导致多个控制块（control block）的创建。

namespace shared_ptr_test
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

struct AStruct;
struct BStruct;

struct AStruct
{
    std::weak_ptr<BStruct> bPtr; // std::shared_ptr<BStruct> bPtr; 如果都用shared_ptr，则循环引用，两个对象都不会释放

    AStruct()
    {
        std::cout << "AStruct is created!" << std::endl;
    }

    ~AStruct()
    {
        std::cout << "AStruct is deleted!" << std::endl;
    }
};

struct BStruct
{
    std::shared_ptr<AStruct> aPtr;

    BStruct()
    {
        std::cout << "BStruct is created!" << std::endl;
    }

    ~BStruct()
    {
        std::cout << "BStruct is deleted!" << std::endl;
    }
};
} // namespace shared_ptr_test

SharedPtrTest::SharedPtrTest()
{

}

SharedPtrTest::~SharedPtrTest()
{

}

void SharedPtrTest::Test001()
{
    std::shared_ptr<shared_ptr_test::Person> p1(new shared_ptr_test::Person(1));// Person(1)的引用计数为1
    std::cout << p1.use_count() << std::endl; // use_count()应该仅仅用于测试或者调试，它不提供高效率的操作，而且有的时候可能是不可用的

    // shared_ptr没有release方法

    std::shared_ptr<shared_ptr_test::Person> p2 = std::make_shared<shared_ptr_test::Person>(2);
    std::cout << p2.use_count() << std::endl;

    p1.reset(new shared_ptr_test::Person(3));// 首先生成新对象，然后对旧对象的引用计数减1，旧对象引用计数为0，故析构Person(1)，最后将新对象的指针交给智能指针，引用计数加1
    std::cout << p1.use_count() << std::endl;

    std::shared_ptr<shared_ptr_test::Person> p3 = p1;//现在p1和p3同时指向Person(3)，Person(3)的引用计数为2
    std::cout << p1.use_count() << std::endl;
    std::cout << p3.use_count() << std::endl;

    std::shared_ptr<shared_ptr_test::Person> p4(p1);//现在p1和p4同时指向Person(3)，Person(3)的引用计数为3
    std::cout << p1.use_count() << std::endl;
    std::cout << p3.use_count() << std::endl;
    std::cout << p4.use_count() << std::endl;

    p1.reset();//Person(3)的引用计数为1
    std::cout << p1.use_count() << std::endl; // use_count为0
    std::cout << p3.use_count() << std::endl;
    std::cout << p4.use_count() << std::endl;

    p3.reset();//Person(3)的引用计数为0，析构Person(3)
    std::cout << p3.use_count() << std::endl;
    std::cout << p4.use_count() << std::endl;

//    注意，不能将一个原始指针直接赋值给一个智能指针，如下所示，原因是一个是类，一个是指针。
//    std::shared_ptr<int> p4 = new int(1); // error

//    没有std::shared_ptr<T[]>.所以shared_ptr只能管理单个对象，而不能管理对象数组。
//    std::shared_ptr<int[]> pArray(new int[3]{1,3,3}); // error

    auto pIn = std::make_shared<shared_ptr_test::Person>(4);// pIn先析构，p2后析构

    std::vector<std::shared_ptr<shared_ptr_test::Person>> m_ptrVec; // 可以用于容器
}

void SharedPtrTest::TestLoopReference()
{
    std::shared_ptr<shared_ptr_test::AStruct> ap(new shared_ptr_test::AStruct);
    std::shared_ptr<shared_ptr_test::BStruct> bp(new shared_ptr_test::BStruct);
    ap->bPtr = bp;
    bp->aPtr = ap;
}

ADD_TEST_F(SharedPtrTest, Test001);
ADD_TEST_F(SharedPtrTest, TestLoopReference);

//https://blog.csdn.net/jxianxu/article/details/72858536
