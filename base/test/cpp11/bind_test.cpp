#include "bind_test.h"
#include <functional>

//bind是一组用于函数绑定的模板。在对某个函数进行绑定时，可以指定部分参数或全部参数，也可以不指定任何参数，还可以调整各个参数间的顺序。
// 对于未指定的参数，可以使用占位符_1、_2、_3来表示。_1表示绑定后的函数的第1个参数，_2表示绑定后的函数的第2个参数，其他依次类推。
//bind可以绑定到普通函数、类的成员函数和类的成员变量

namespace bind_test
{
class A
{
public:
    A()
    {
        a = 2;
    }

    ~A() = default;

    void fun_3(int k, int m)
    {
        std::cout << k << " " << m << std::endl;
    }

    int a;
};

void fun(int x, int y, int z)
{
    std::cout << x << "  " << y << "  " << z << std::endl;
}

void fun_2(int& a, int& b)
{
    a++;
    b++;
    std::cout << a << "  " << b << std::endl;
}

void f(int& n1, int& n2, const int& n3)
{
    std::cout << "In function: " << n1 << ' ' << n2 << ' ' << n3 << '\n';
    ++n1; // increments the copy of n1 stored in the function object
    ++n2; // increments the main()'s n2
    // ++n3; // compile error
}
}

BindTest::BindTest()
{

}

BindTest::~BindTest()
{

}

void BindTest::Test001()
{
    auto f1 = std::bind(bind_test::fun, 1, 2, 3); //表示绑定函数 fun 的第一，二，三个参数值为： 1 2 3
    f1(); //print:1  2  3

    auto f2 = std::bind(bind_test::fun, std::placeholders::_1, std::placeholders::_2, 3);
    //表示绑定函数 fun 的第三个参数为 3，而fun 的第一，二个参数分别由调用 f2 的第一，二个参数指定
    f2(1, 2);//print:1  2  3

    auto f3 = std::bind(bind_test::fun, std::placeholders::_2, std::placeholders::_1, 3);
    //表示绑定函数 fun 的第三个参数为 3，而fun 的第一，二个参数分别由调用 f3 的第二，一个参数指定
    //注意： f2  和  f3 的区别。
    f3(1, 2);//print:2  1  3


    int n = 2;
    int m = 3;

    auto f4 = std::bind(bind_test::fun_2, n, std::placeholders::_1);
    f4(m); //print:3  4

    std::cout << m << std::endl;//print:4  说明：bind对于不事先绑定的参数，通过std::placeholders传递的参数是通过引用传递的
    EXPECT_EQ(4, m);
    std::cout << n << std::endl;//print:2  说明：bind对于预先绑定的函数参数是通过值传递的
    EXPECT_EQ(2, n);

    bind_test::A a;
    auto f5 = std::bind(&bind_test::A::fun_3, a, std::placeholders::_1, std::placeholders::_2);
    f5(10, 20);//print:10 20

    auto f6 = std::bind(&bind_test::A::fun_3, std::placeholders::_3, std::placeholders::_1, std::placeholders::_2);
    f6(10, 20, a);//print:10 20

    auto f7 = std::bind(&bind_test::A::a, std::placeholders::_1);
    std::cout << f7(a) << std::endl;

    std::function<void(int, int)> fc = std::bind(&bind_test::A::fun_3, a, std::placeholders::_1, std::placeholders::_2);
    fc(10, 20);//print:10 20
}

void BindTest::Test002()
{
    int n1 = 1, n2 = 2, n3 = 3;
    std::function<void()> bound_f = std::bind(bind_test::f, n1, std::ref(n2),
                                              std::cref(n3)); // 对直接bind的参数，是传值方式，在绑定的时候就确定了。可以通过std::ref改为引用方式
    n1 = 10;
    n2 = 11;
    n3 = 12;
    std::cout << "Before function: " << n1 << ' ' << n2 << ' ' << n3 << '\n';
    bound_f();
    std::cout << "After function: " << n1 << ' ' << n2 << ' ' << n3 << '\n';
}

ADD_TEST_F(BindTest, Test001);
ADD_TEST_F(BindTest, Test002);
