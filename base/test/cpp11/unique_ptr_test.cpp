#include "unique_ptr_test.h"

//unique_ptr“唯一”拥有其所指对象，同一时刻只能有一个unique_ptr指向给定对象（通过禁止拷贝语义、只有移动语义来实现）。
//相比与原始指针unique_ptr用于其RAII的特性，使得在出现异常的情况下，动态资源能得到释放。
//unique_ptr指针本身的生命周期：从unique_ptr指针创建时开始，直到离开作用域。离开作用域时，若其指向对象，
//则将其所指对象销毁(默认使用delete操作符，用户可指定其他操作)。
//unique_ptr指针与其所指对象的关系：在智能指针生命周期内，可以改变智能指针所指对象，
// 如创建智能指针时通过构造函数指定、通过reset方法重新指定(会自动释放之前管理的旧对象）、
// 通过release方法释放所有权、通过移动语义转移所有权。

//成员函数
//(1) get 获得内部对象的指针, 由于已经重载了()方法, 因此和直接使用对象是一样的.如 unique_ptr<int> sp(new int(1)); sp 与 sp.get()是等价的
//(2) release 放弃内部对象的所有权，将内部指针置为空, 返回所内部对象的指针, 此指针需要手动释放
//(3) reset 销毁内部对象并接受新的对象的所有权(如果使用缺省参数的话，也就是没有任何对象的所有权, 此时仅将内部对象释放, 并置为空)
//(4) swap 交换两个 shared_ptr 对象(即交换所拥有的对象)
//std::move(up) 所有权转移(通过移动语义), up所有权转移后，变成“空指针” (up 的定义为 std::unique_ptr<Ty> up)

//unique_ptr 不支持拷贝和赋值.
//　　std::unique_ptr<A> up1(new A(5));
//　　std::unique_ptr<A> up2(up1); // 错误, unique_ptr 不支持拷贝
//　　std::unique_ptr<A> up2 = up1; // 错误, unique_ptr 不支持赋值

//虽然 unique_ptr 不支持拷贝和赋值, 但是我们可以调用 release 或 reset 将指针的所有权从一个(非 const) unique_ptr 转移到另一个.
//　　std::unique_ptr<int> up1(new int(1));
//　　std::unique_ptr<int> up2(up1.release());

//虽然 unique_ptr 不支持拷贝, 但是可以从函数中返回, 甚至返回局部对象. 如下面的代码, 编译器知道要返回的对象即将被销毁, 因此执行一种特殊的"拷贝":
//　　template <class Ty>
//　　std::unique_ptr<Ty> Clone(const Ty& obj)
//　　{
//　　　　return std::unique_ptr<Ty>(new Ty(obj));
//　　}

//　　template <class Ty>
//　　std::unique_ptr<Ty> Clone(const Ty& obj)
//　　{
//　　　　std::unique_ptr<Ty> temp = std::unique_ptr<Ty>(new Ty(obj));
//　　　　return temp;
//　　}

// get()方法返回所管理的对象指针
// unique_ptr不能拷贝和赋值
// 可以管理动态数组，也可以作为stl容器元素（通过std::move()转移所有权），但不能管理stl容器
// 可以将shared_ptr置为null，会释放其管理的对象

//unique_ptr使用场景
//1、在函数内部等地方为动态申请的资源提供异常安全保证
//2、返回函数内动态申请资源的所有权
//3、在stl容器中保存指针
//4、管理动态数组
//5、作为auto_ptr的替代品

namespace unique_ptr_test
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

std::unique_ptr<int> clone(int p)
{
    std::unique_ptr<int> up(new int(p));
    return up;    // 返回unique_ptr
}

struct Foo
{
    Foo()
    {
        std::cout << "Foo::Foo\n";
    }
    ~Foo()
    {
        std::cout << "Foo::~Foo\n";
    }
    void bar()
    {
        std::cout << "Foo::bar\n";
    }
};

void f(const Foo&)
{
    std::cout << "f(const Foo&)\n";
}

struct D
{
    void operator()(Foo* foo)
    {
        std::cout << "D operator()" << std::endl;
        delete foo;
    }
};
}

UniquePtrTest::UniquePtrTest()
{

}

UniquePtrTest::~UniquePtrTest()
{

}

void UniquePtrTest::Test001()
{
    {
        std::unique_ptr<unique_ptr_test::Person> uptr(new unique_ptr_test::Person(1));  //绑定动态对象
        EXPECT_NE(nullptr, uptr.get());
        //std::unique_ptr<unique_ptr_test::Person> uptr2 = uptr;  //不能賦值
        //std::unique_ptr<unique_ptr_test::Person> uptr2(uptr);  //不能拷貝   注意：unique_ptr可以作为函数的返回值
        std::unique_ptr<unique_ptr_test::Person> uptr2 = std::move(uptr); //转移所有权，转移后，uptr不再管理对象
        EXPECT_EQ(nullptr, uptr.get());

        unique_ptr_test::Person* p = uptr2.release(); //释放智能指针的所有权，relese之后必须手工释放对象p，否则内存泄露
        EXPECT_EQ(nullptr, uptr2.get());

        delete p; // 手工释放对象

        std::unique_ptr<unique_ptr_test::Person> uptr3(new unique_ptr_test::Person(3));  //绑定动态对象

        uptr3.reset(new unique_ptr_test::Person(4));    //"绑定”另一个动态对象，会自动释放之前的对象
        EXPECT_NE(nullptr, uptr3.get());
    }
    //超過uptr的作用域，內存釋放


//    由于unique_ptr有std::unique_ptr<T[]>的重载函数，所以它可以用来管理数组资源
    std::unique_ptr<int[]> pa(new int[3] {1, 3, 3});

    std::unique_ptr<int> up;     // 创建一个空的智能指针
    EXPECT_EQ(nullptr, up.get());

    //    注意，不能将一个原始指针直接赋值给一个智能指针，如下所示，原因是一个是类，一个是指针。
//    std::unique_ptr<int> p4 = new int(1);// error

    std::unique_ptr<int> sp(new int(88));
    std::vector<std::unique_ptr<int>> vec;
    vec.push_back(std::move(sp)); // 转移
//    vec.push_back(sp); // error unique_ptr不可拷贝

    {
        // unique_ptr置为null测试
        std::unique_ptr<unique_ptr_test::Person> uptr(new unique_ptr_test::Person(5));  //绑定动态对象
        uptr = nullptr; // 将unique_ptr置位null，会自动释放其所管理的对象

        const int n = 10;
        for (int i = 0; i < n; ++i)
        {
            ;
        }
    }
}

void UniquePtrTest::Test002()
{
//    unique_ptr不支持拷贝操作，但却有一个例外：可以从函数中返回一个unique_ptr。用于返回函数内动态申请资源的所有权
    int p = 5;
    std::unique_ptr<int> ret = unique_ptr_test::clone(p);
    std::cout << *ret << std::endl;
}

void UniquePtrTest::Test003()
{
//  在容器中保存指针
    std::vector<std::unique_ptr<int>> vec;
    std::unique_ptr<int> p(new int(5));
    vec.push_back(std::move(p));    // 使用移动语义
}

void UniquePtrTest::Test004()
{
//    标准库提供了一个可以管理动态数组的unique_ptr版本。数组元素可以是自定义类型
    std::unique_ptr<int[]> p(new int[5] {1, 2, 3, 4, 5});
    p[0] = 0;   // 重载了operator[]
}

void UniquePtrTest::TestAutoDestroy()
{
    //1. 普通的new对象.
    std::cout << "TestDestroy...................." << std::endl;
    {
        std::unique_ptr<unique_ptr_test::Foo> p1(new unique_ptr_test::Foo);
    }
    //2. 普通的new[]对象.
    {
        std::unique_ptr<unique_ptr_test::Foo[]> p2(new unique_ptr_test::Foo[4]);
    }
    //3. 自定义的deleter.
    {
        std::unique_ptr<unique_ptr_test::Foo, unique_ptr_test::D> p3(new unique_ptr_test::Foo);
    }
}

void UniquePtrTest::TestOwner()
{
    std::cout << "TestOwner...................." << std::endl;
    //1. new object.
    std::unique_ptr<unique_ptr_test::Foo> p1(new unique_ptr_test::Foo);  // p1 owns Foo
    if (p1)
    {
        p1->bar();
    }

    {
        std::unique_ptr<unique_ptr_test::Foo> p2(std::move(p1));  // now p2 owns Foo
        f(*p2);

        p1 = std::move(p2);  // ownership returns to p1
        p2->bar();
        std::cout << "destroying p2...\n";
    }

    p1->bar();
}

void UniquePtrTest::TestArrayOwner()
{
    std::cout << "TestArrayOwner...................." << std::endl;
    //1. new[] object.
    std::unique_ptr<unique_ptr_test::Foo[]> p1(new unique_ptr_test::Foo[4]);  // p1 owns Foo
    if (p1)
    {
        p1[0].bar();
    }

    {
        std::unique_ptr<unique_ptr_test::Foo[]> p2(std::move(p1));  // now p2 owns Foo
        f(p2[0]);

        p1 = std::move(p2);  // ownership returns to p1
        p2[0].bar();
        std::cout << "destroying p2...\n";
    }

    p1[0].bar();
}

ADD_TEST_F(UniquePtrTest, Test001);
ADD_TEST_F(UniquePtrTest, Test002);
ADD_TEST_F(UniquePtrTest, Test003);
ADD_TEST_F(UniquePtrTest, Test004);
ADD_TEST_F(UniquePtrTest, TestAutoDestroy);
ADD_TEST_F(UniquePtrTest, TestOwner);
ADD_TEST_F(UniquePtrTest, TestArrayOwner);
