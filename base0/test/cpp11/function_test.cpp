#include "function_test.h"
#include <functional>

//类模板std:function定义在functional头文件中，是一种通用的多态函数包装器，std:function的实例可以存储、复制、调用任何可调用对象。
//在C++中，可调用对象包括普通函数、函数指针、Lambda表达式、bind创建的函数以及其它函数对象。std:function是对C++可调用对象的一种类型安全的包装，
//与函数指针相比，std:function的目标既可以是普通函数，也可以是函数对象和类的成员对象。

//function是一个template，在一对尖括号中制定类型：function<int(int, int)> ，表示声明一个function类型，接受两个int、返回一个int的可调用对象

namespace function_test
{
int add(int x, int y)
{
    return x + y;
}

class Num
{
public:
    void operator()(int i)
    {
        std::cout << "here i = " << i << std::endl;
    }
};

class A
{
public:
    int add(int x, int y)
    {
        return x + y;
    }
};

struct Foo
{
    Foo(int num) : num_(num)
    {
    }

    void print_add(int i) const
    {
        std::cout << num_ + i << '\n';
    }

    int num_;
};

void print_num(int i)
{
    std::cout << i << '\n';
}

struct PrintNum
{
    void operator()(int i) const
    {
        std::cout << i << '\n';
    }
};
}

FunctionTest::FunctionTest()
{

}

FunctionTest::~FunctionTest()
{

}

/**
 * @brief 普通函数
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void FunctionTest::Test001()
{
    std::function<int(int, int)> fun = function_test::add;
    std::cout << fun(1, 9) << std::endl;  // 输出10
}

/**
 * @brief 函数对象
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void FunctionTest::Test002()
{
    function_test::Num num;
    std::function<void(int)> fun = num;
    fun(10);  // 输出 here i = 10
}

/**
 * @brief 类的成员函数
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void FunctionTest::Test003()
{
    function_test::A a;
    std::function<int(function_test::A&, int, int)> func = &function_test::A::add;
    std::cout << func(a, 1, 9) << std::endl;
}

void FunctionTest::Test004()
{
    // store a free function
    std::function<void(int)> f_display = function_test::print_num;
    f_display(-9);

    // store a lambda
    std::function<void()> f_display_42 = []() { function_test::print_num(42); };
    f_display_42();

    // store the result of a call to std::bind
    std::function<void()> f_display_31337 = std::bind(function_test::print_num, 31337);
    f_display_31337();

    // store a call to a member function
    std::function<void(const function_test::Foo&, int)> f_add_display = &function_test::Foo::print_add;
    const function_test::Foo foo(314159);
    f_add_display(foo, 1);

    // store a call to a data member accessor
    std::function<int(function_test::Foo const&)> f_num = &function_test::Foo::num_;
    std::cout << "num_: " << f_num(foo) << '\n';

    // store a call to a member function and object
    using std::placeholders::_1;
    std::function<void(int)> f_add_display2 = std::bind(&function_test::Foo::print_add, foo, _1);
    f_add_display2(2);

    // store a call to a member function and object ptr
    std::function<void(int)> f_add_display3 = std::bind(&function_test::Foo::print_add, &foo, _1);
    f_add_display3(3);

    // store a call to a function object
    std::function<void(int)> f_display_obj = function_test::PrintNum();
    f_display_obj(18);

//    在实际使用中都用 auto 关键字来代替std::function… 这一长串了。
}
