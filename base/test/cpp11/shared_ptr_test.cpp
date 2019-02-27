#include "shared_ptr_test.h"
#include <memory>

//多个shared_ptr可以指向同一份资源。
// shared_ptr使用引用计数，每一个shared_ptr的拷贝都指向相同的内存。每使用他一次，内部的引用计数加1，
//每析构一次，内部的引用计数减1，减为0时，自动释放所指向的堆内存。
//shared_ptr内部的引用计数是线程安全的，但是所指向对象的读取需要加锁。

//初始化
//　　可以通过构造函数、std::make_shared<T>辅助函数和reset方法来初始化shared_ptr：
//　  reset()包含两个操作。当智能指针中有值的时候，调用reset()会使引用计数减1.
//    当调用reset（new xxx())重新赋值时，智能指针首先是生成新对象，然后将旧对象的引用计数减1
//（当然，如果发现引用计数为0时，则析构旧对象），然后将新对象的指针交给智能指针保管。

//获取原始指针：get函数
//std::shared_ptr<int> p4(new int(5));
//int *pInt = p4.get();

//指定删除器
//　　智能指针可以指定删除器，当智能指针的引用计数为0时，自动调用指定的删除器来释放内存。
//std::shared_ptr可以指定删除器的一个原因是其默认删除器不支持数组对象


//shared_ptr有多种形式的构造函数，应用于各种可能的情形：
//
//▲ 无参的shared_ptr( )创建一个持有空指针的shared_ptr;
//
//▲ shared_ptr(Y *p)获得指向类型T的指针p的管理权，同时引用计数置为1。
//这个构造函数要求Y类型必须能够转换为T类型；
//
//▲ shared_ptr(shared_ptr const & r)从另外一个shared_ptr获得指针的管理权，
//同时引用计数加1，结果是两个shared_ptr共享一个指针的管理权；
//
//▲ shared_ptr(std::auto_ptr<Y> & r)从一个auto_ptr获得指针的管理权，引用计数置为1，
//同时auto_ptr自动失去管理权；
//
//▲ operator=赋值操作符可以从另外一个shared_ptr或auto_ptr获得指针的管理权，其行为同构造函数；
//
//▲ shared_ptr( Y *p, D d)行为类似shared_ptr（Y * p)，但使用参数d指定了析构时的定制删除器，
//而不是简单的delete。
//
//
//shared_ptr还支持比较运算符，可以测试两个shared_ptr相等或者不等，比较基于内部保存的指针，
//相当于a.get( ) == b.get( )。
//
//shared_ptr还可以使用operator<比较大小，同样基于内部保存的指针，但不提供除operator<以外的比较操作符，
//这使得shared_ptr可以被用于标准关联容器(set 和 map):
//
//shared_ptr还支持流输出操作符operator<<，输出内部的指针值，方便调试。
//
//shared_ptr有两个专门的函数检查引用计数。unique()在shared_ptr是指针的唯一拥所有者时返回true。
//use_count( )返回当前的引用计数。


//使用shared_ptr需要注意的问题
//
//1. 不要用一个原始指针初始化多个shared_ptr，原因在于，会造成二次销毁，如下所示：
//int *p5 = new int;
//std::shared_ptr<int> p6(p5);
//std::shared_ptr<int> p7(p5);// logic error
//
//2. 不要在函数实参中创建shared_ptr。因为C++的函数参数的计算顺序在不同的编译器下是不同的。
//正确的做法是先创建好，然后再传入。
//function(shared_ptr<int>(new int), g());
//
//3. 避免循环引用。智能指针最大的一个陷阱是循环引用，循环引用会导致内存泄漏。
//解决方法是AStruct或BStruct改为weak_ptr。

//相比于普通指针，共享指针要占用多一倍的内存空间，其内部包含两个指针，一个指针指向它所管理的资源，
//第二个指针指向一个称为“Control Block”的控制块
//在控制块中有一个引用计数字段（Reference Count），用来记录指向Object的共享指针数量，
//当我们声明并初始化一个共享指针时，Reference Count的值为1，
//当有另一个共享指针指向该Object时（比如通过赋值或拷贝构造函数，将一个共享指针赋值给另一共享指针），
//Reference count 的值递增1。
//第二个字段：Weak Count，也是一个引用计数，它用来计数指向该Object的std::weak_ptr指针的数量。

//资源释放器的地址存放于控制块（Control Block）中，因此增加用户自定义的资源释放器不会增加shared_ptr的大小，
//其大小仍然为两个指针的大小。
//还有Allocator的地址也在控制块中。


//当代码第一次为一个对象创建shared_ptr指针指向这个对象的时候，控制块就会被创建，或者说它应该被创建。
//但是问题在于，一处代码在创建shared_ptr来指向一个对象时，这处代码并不知道它创建的这个shared_ptr
//所要指向的对象是否已经有别的shared_ptr指向了，即这个对象是否已经有一个控制块与之对应了。
//为了解决这个问题，控制块的创建遵循以下原则：
//
//std::make_shared总是会创建一个控制块。这是因为当调用std::make_shared创建智能指针的时候，
//智能指针所要指向的这个对象是第一次被创建的。
//
//当通过一个普通指针作为参数构建一个智能指针的时候，会为这个指针所指向的对象创建一个控制块。
//
//在这种情况下，如果用户想要为一个已经有控制块的对象创建一个shared_ptr时，需要用shared_ptr或者weak_ptr
//作为共享指针构造函数的参数，而不能用普通指针作为参数。
//
//注意：使用普通指针作为参数构造一个shared_ptr有可能会导致多个控制块（control block）的创建。

//操作    解释
//shared_ptr< T > sp    创建空智能指针，可以指向T类型的对象
//sp.get()  返回sp中保存的指针。PS：一定要小心使用，如果智能指针释放了对象，返回的指针指向的对象也将消失
//sp.reset()    若sp是唯一指向该对象的shared_ptr,reset释放该对象
//sp.reset(p)   sp不在指向原来它指向的对象，指向内置指针p指向的对象,这里p是被new动态分配内存的
//sp.reset(p,d) sp不在指向原来它指向的对象，指向内置指针p指向的对象,这里p将会被可调用对象d释放
//sp.use_count()    返回sp指向对象的引用计数
//sp.unique()   如果sp.use_count() == 1，返回true否则返回false
//swap(sp1,sp2) 交换两个智能指针
//shared_ptr< T > sp(p) sp管理内置指针p，p必须是被new动态分配内存的，而且能转换为T*类型
//shared_ptr< T > sp(p,d)   sp接管内置指针p指向对象的所有权，p必须能准换为T*类型，sp将使用可调用对象d代替delete
//shared_ptr< T > sp1(sp2,d)    sp1是shared_ptr sp2的拷贝，使用可调用对象d代替delete

// shared_ptr没有release方法
// shared_ptr不能管理动态数组和stl容器，但可以作为stl容器元素（拷贝构造）
//shared_ptr可以当做函数的参数，也可以当做函数的返回值，这时候相当于使用拷贝构造
// 可以将shared_ptr置为null，当引用计数为1时会释放其管理的对象，大于1时不会

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
    std::weak_ptr<BStruct> b_ptr; // std::shared_ptr<BStruct> bPtr; 如果都用shared_ptr，则循环引用，两个对象都不会释放

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
    std::shared_ptr<AStruct> a_ptr;

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

    std::shared_ptr<shared_ptr_test::Person> p2 = std::make_shared<shared_ptr_test::Person>(2);
    std::cout << p2.use_count() << std::endl;

    p1.reset(new shared_ptr_test::Person(3));// 首先生成新对象，然后旧对象的引用计数减1，旧对象引用计数为0，故析构Person(1)，最后将新对象的指针交给智能指针，引用计数加1
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

    auto p5 = std::make_shared<shared_ptr_test::Person>(4);// p5先析构，p2后析构

    auto p6 = std::make_shared<shared_ptr_test::Person>(6);
    p6 = nullptr; // 引用计数为1，置为null时释放管理的对象。p6.reset()也是一样的效果

    auto p7 = std::make_shared<shared_ptr_test::Person>(7);
    std::shared_ptr<shared_ptr_test::Person> p8(p7);
    p7 = nullptr; // 引用计数大于1，置为null时不会释放管理的对象。p7.reset()也是一样的效果

    std::vector<std::shared_ptr<shared_ptr_test::Person>> shared_ptr_vec; // 可以用于容器元素
}

void SharedPtrTest::TestLoopReference()
{
    std::shared_ptr<shared_ptr_test::AStruct> ap(new shared_ptr_test::AStruct);
    std::shared_ptr<shared_ptr_test::BStruct> bp(new shared_ptr_test::BStruct);
    ap->b_ptr = bp;
    bp->a_ptr = ap;
}

//std::enable_shared_from_this 有什么意义?(在类的内部获得自己的shared_ptr,自引用。背景：所有的指针都用智能指针管理起来，不使用原始指针)
// 使用场景：在类中发起一个异步操作, callback回来要保证发起操作的对象仍然有效.
// struct A {
//void func() {
//  // only have "this" ptr ?
//}
//};

//int main() {
//A* a;
//std::shared_ptr<A> sp_a(a);
//}
//当A* a被shared_ptr托管的时候,如何在func获取自身的shared_ptr成了问题.如果写成:void func() {
//std::shared_ptr<A> local_sp_a(this);
//// do something with local_sp_a
//}
//又用a新生成了一个shared_ptr: local_sp_a, 这个在生命周期结束的时候可能将a直接释放掉.

//对于通过智能指针管理的类，在类中通过shared_from_this()而不是this来传递本身
//在类释放时，尽量手动置空其所有的shared_ptr成员，包括function

namespace shared_ptr_test
{
class tester : public std::enable_shared_from_this<tester>
{
public:
    tester() {}
    ~tester()
    {
        std::cout << "析构函数被调用!\n";
    }
public:
    std::shared_ptr<tester> sget()
    {
        return shared_from_this(); // shared_from_this会从weak_ptr安全的生成一个自身的shared_ptr.
    }
};
}
void SharedPtrTest::TestShareThisPointer()
{
    {
        std::shared_ptr<shared_ptr_test::tester> sp(new shared_ptr_test::tester);
        std::shared_ptr<shared_ptr_test::tester> sp2 = sp->sget();

        std::cout << "sp.use_count() = " << sp.use_count() << std::endl;
        std::cout << "sp2.use_count() = " << sp2.use_count() << std::endl;
    }
}

ADD_TEST_F(SharedPtrTest, Test001);
ADD_TEST_F(SharedPtrTest, TestLoopReference);
ADD_TEST_F(SharedPtrTest, TestShareThisPointer);

//https://blog.csdn.net/jxianxu/article/details/72858536
