#include "lambda_test.h"
#include <functional>

//利用lambda表达式可以编写内嵌的匿名函数，用以替换独立函数或者函数对象
//在类中，可以重载函数调用运算符()，此时类的对象具有类似函数的行为，我们称这些对象为函数对象（Function Object）或者仿函数（Functor）。
// 相比lambda表达式，函数对象有自己独特的优势。

// 闭包就是一个引用了父环境的对象，并且从父环境中返回到更高层的环境中的一个对象。
// 换一种说法：如果一个函数引用了父环境中的对象，并且在这个函数中把这个对象返回到了更高层的环境中，那么，这个函数就是闭包。

//什么是闭包?
//
//简单来说，闭包是指可以访问另一个函数作用域变量的函数，一般是定义在外层函数中的内层函数。
//
//为什么需要闭包？
//
//局部变量无法共享和长久的保存，而全局变量可能造成变量污染，所以我们希望有一种机制既可以长久的保存变量又不会造成全局污染。
//
//特点
//
//    占用更多内存
//不容易被释放
//    何时使用？
//
//变量既想反复使用，又想避免全局污染
//
//    如何使用？
//
//定义外层函数，封装被保护的局部变量。
//定义内层函数，执行对外部函数变量的操作。
//外层函数返回内层函数的对象，并且外层函数被调用，结果保存在一个全局的变量中。

LambdaTest::LambdaTest()
{

}

LambdaTest::~LambdaTest()
{

}

void LambdaTest::Test001()
{
//    lambda表达式一般都是从方括号[]开始，然后结束于花括号{}，花括号里面就像定义函数那样，包含了lamdba表达式体：
    // 定义简单的lambda表达式，没有参数
    auto BasicLambda = [] { std::cout << "Hello, world!" << std::endl; };
    // 调用
    BasicLambda();   // 输出：Hello, world!

}

void LambdaTest::Test002()
{
//    如果需要参数，那么就要像函数那样，放在圆括号里面，如果有返回值，返回类型要放在->后面，即拖尾返回类型，当然你也可以忽略返回类型，lambda会帮你自动推断出返回类型：
    // 指明返回类型
    auto add = [](int a, int b) -> int { return a + b; };
    // 自动推断返回类型
    auto multiply = [](int a, int b)
    {
        return a * b;
    };

    int sum = add(2, 5);   // 输出：7
    EXPECT_EQ(7, sum);
    int product = multiply(2, 5);  // 输出：10
    EXPECT_EQ(10, product);
}

void LambdaTest::Test003()
{
//    lambda表达式最前面的方括号是lambda表达式一个很重要的功能闭包。
//    lambda表达式的大致原理：每当你定义一个lambda表达式后，编译器会自动生成一个匿名类（这个类当然重载了()运算符），我们称为闭包类型（closure type）。
//    那么在运行时，这个lambda表达式就会返回一个匿名的闭包实例，其是s一个右值。
//    闭包的一个强大之处是其可以通过传值或者引用的方式捕捉其封装作用域内的变量，前面的方括号就是用来定义捕捉模式以及变量，我们又将其称为lambda捕捉块。
//    当lambda捕捉块为空时，表示没有捕捉任何变量

    int x = 10;

    auto add_x = [x](int a)
    {
        return a + x;
    };  // 复制捕捉x
    auto multiply_x = [&x](int a)
    {
        return a * x;
    };  // 引用捕捉x

    std::cout << add_x(10) << " " << multiply_x(10) << std::endl;
    // 输出：20 100
}

void LambdaTest::Test004()
{
//    对于复制传值捕捉方式，类中会相应添加对应类型的非静态数据成员。在运行时，会用复制的值初始化这些成员变量，从而生成闭包。
//    lambda表达式无法修改通过复制形式捕捉的变量，想改动传值方式捕获的值，那么就要使用mutable：
//    对于引用捕获方式，无论是否标记mutable，都可以在lambda表达式中修改捕获的值。至于闭包类中是否有对应成员，C++标准中给出的答案是：不清楚的，看来与具体实现有关
    int x = 10;

    auto add_x = [x](int a) mutable { x *= 2; return a + x; };  // 复制捕捉x

    std::cout << add_x(10) << std::endl; // 输出 30
}

void LambdaTest::Test005()
{
    auto a = [] { std::cout << "A" << std::endl; };
    auto b = [] { std::cout << "B" << std::endl; };
    (void) b;

//    a = b;   // 非法，lambda无法赋值，因为禁用了赋值操作符
    auto c = a;   // 合法，生成一个副本
    c();

//    lambda表达式也可以赋值给相对应的函数指针，这也使得你完全可以把lambda表达式看成对应函数类型的指针。
    typedef void (*f)();
    f f1 = a;
    f1();
}

void LambdaTest::Test006()
{
//    捕获的方式可以是引用也可以是复制，但是具体说来会有以下几种情况来捕获其所在作用域中的变量：
//
//    []：默认不捕获任何变量；
//    [=]：默认以值捕获所有变量；
//    [&]：默认以引用捕获所有变量；
//    [x]：仅以值捕获x，其它变量不捕获；
//    [&x]：仅以引用捕获x，其它变量不捕获；
//    [=, &x]：默认以值捕获所有变量，但是x是例外，通过引用捕获；
//    [&, x]：默认以引用捕获所有变量，但是x是例外，通过值捕获；
//    [this]：通过引用捕获当前对象（其实是复制指针）；
//    [*this]：通过传值方式捕获当前对象；
//
//    在上面的捕获方式中，注意最好不要使用[=]和[&]默认捕获所有变量。

//    默认引用捕获所有变量，你有很大可能会出现悬挂引用（Dangling references），因为引用捕获不会延长引用的变量的声明周期：
//    std::function<int(int)> add_x(int x)
//    {
//        return [&](int a) { return x + a; };
//    }
    //    因为参数x仅是一个临时变量，函数调用后就被销毁，但是返回的lambda表达式却引用了该变量，但调用这个表达式时，引用的是一个垃圾值，所以会产生没有意义的结果。


//    采用默认值捕获所有变量仍然有风险
    class Filter
    {
    public:
        Filter(int divisorVal):
            divisor{divisorVal}
        {}

        // lambda表达式实际上捕捉的是this指针的副本，所以原来的代码等价于：
        std::function<bool(int)> getFilter()
        {
            return [ = ](int value)
            {
                return value % divisor == 0;
            };
        }

//        std::function<bool(int)> getFilter()
//        {
//            return [this](int value) {return value % this->divisor == 0; };
//        }

    private:
        int divisor;
    };

//    尽管还是以值方式捕获，但是捕获的是指针，其实相当于以引用的方式捕获了当前类对象，所以lambda表达式的闭包与一个类对象绑定在一起了，这也很危险，
//    因为你仍然有可能在类对象析构后使用这个lambda表达式，那么类似“悬挂引用”的问题也会产生。

}

ADD_TEST_F(LambdaTest, Test001);
ADD_TEST_F(LambdaTest, Test002);
ADD_TEST_F(LambdaTest, Test003);
ADD_TEST_F(LambdaTest, Test004);
ADD_TEST_F(LambdaTest, Test005);
