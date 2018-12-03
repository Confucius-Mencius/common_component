#include "auto_test.h"

//C++引入auto关键字主要有两种用途：一是在变量声明时根据初始化表达式自动推断该变量的类型，二是在声明函数时作为函数返回值的占位符。
//对于自动类型推断，C++11中也有一个类似的关键字decltype

namespace auto_test
{
template<class T, class U>
void add(T t, U u)
{
    auto s = t + u;
    std::cout << "type of t + u is " << typeid(s).name() << std::endl;
}

template<class T, class U>
auto add2(T t, U u) -> decltype(t + u)
{
    return t + u;
}
} // namespace auto_test

AutoTest::AutoTest()
{

}

AutoTest::~AutoTest()
{

}

/**
 * @brief
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention
使用auto关键字的变量必须有初始值。
可以使用valatile，*（指针类型说明符），&（引用类型说明符），&&（右值引用）来修饰auto关键字。
函数参数和模板参数不能被声明为auto。
 */
void AutoTest::Test001()
{
    // 简单自动类型推断
    auto i = 123;
    std::cout << "type of a is " << typeid(i).name() << std::endl;
    auto s("fred");
    std::cout << "type of s is " << typeid(s).name() << std::endl;

    // 冗长的类型说明（如迭代器）
    std::vector<int> vec;
    auto iter = vec.begin();
    std::cout << "type of iter is " << typeid(iter).name() << std::endl;

    // 使用模板技术时，如果某个变量的类型依赖于模板参数，使用auto确定变量类型
    auto_test::add(101, 1.1);

    auto a = 10;
    auto* pa = new auto(a);
    auto** rpa = new auto(&a);
    std::cout << typeid(a).name() << std::endl;   // 输出： int
    std::cout << typeid(pa).name() << std::endl;  // 输出： int *
    std::cout << typeid(rpa).name() << std::endl; // 输出： int **
}

/**
 * @brief 函数返回值占位符
 * @details 
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention
auto主要与decltype关键字配合使用，作为返回值类型后置时的占位符。此时，关键字不表示自动类型检测，仅仅是表示后置返回值的语法的一部分。
 */
void AutoTest::Test002()
{
    std::cout << auto_test::add2(1, 2) << std::endl;
}

/**
 * @brief 使用auto关键字进行类型推导时，如果初始化表达式是引用类型，编译器会去除引用，除非显示声明
 * @details 
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void AutoTest::Test003()
{
    int i = 10;
    int& r = i;
    auto a = r;
    a = 13; // 重新赋值
    std::cout << "i = " << i << " a = " << a << std::endl;    // 输出i=10，a=13

    // 显式声明
    auto& b = r;
    b = 15; // 重新赋值
    std::cout << "i = " << i << " b = " << b << std::endl;    // 输出i=15，a=15
}

/**
 * @brief 使用auto关键字进行类型推导时，编译器会自动忽略顶层const，除非显示声明
 * @details 
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void AutoTest::Test004()
{
    const int c1 = 10;
    auto c2 = c1;
//    c1 = 11; // 报错，c1为const int类型，无法修改const变量
    c2 = 14; // 正确，c2为int类型

    // 显示声明
    const auto c3 = c1;
    (void) c3;
//    c3 = 15; // 报错，c3为const int类型，无法修改const变量
}

/**
 * @brief 对于数组类型，auto关键字会推导为指针类型，除非被声明为引用
 * @details c
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void AutoTest::Test005()
{
    int a[10];
    auto b = a;
    std::cout << typeid(b).name() << std::endl;   // 输出：int *

    auto& c = a;
    std::cout << typeid(c).name() << std::endl;   // 输出：int [10]
}
