#include <vector>
#include <map>
#include "test_util.h"

// 全特化
// general version
template<class T>
class Compare
{
public:
    static bool IsEqual(const T& lhs, const T& rhs)
    {
        return lhs == rhs;
    }
};

// specialize for float
template<>
class Compare<float>
{
public:
    static bool IsEqual(const float& lhs, const float& rhs)
    {
        return abs(lhs - rhs) < 10e-3;
    }
};

// 偏特化
template<class T1, class T2>
class A
{
};

template<class T1>
class A<T1, int>
{
};

// 以下是各种特化种类
// 特化为引用，指针类型：
// specialize for T*
template<class T>
class Compare<T*>
{
public:
    static bool IsEqual(const T* lhs, const T* rhs)
    {
        return Compare<T>::IsEqual(*lhs, *rhs);
    }
};

// 特化为另外一个类模板：
// specialize for vector<T>
template<class T>
class Compare<std::vector<T> >
{
public:
    static bool IsEqual(const std::vector<T>& lhs, const std::vector<T>& rhs)
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }
        else
        {
            for (int i = 0; i < lhs.size(); ++i)
            {
                if (lhs[i] != rhs[i])
                {
                    return false;
                }
            }
        }
        return true;
    }
};

// 混合型的：
template<typename T1, typename T2>
class X
{
};

template<typename T>
class X<std::vector<T>, int&>
{
};
//至于这里怎么都把T2搞没了变成只依赖一个模板参数T了的问题，大家别着急，我来告诉你个本质的东西，把握这么三点就可以了：1.模板参数个数一 致；2.只要template <...>里面有东西不是<>，比如typename T，那么特化时就得用到T；3.不进行任何对模板参数的修饰也是不行的，比如template<typename T> class<T>{...}，至少你也得搞个const T之类的吧，呵呵。下面是我搞出来的几种特殊情况，它们都是正确的：

template<typename T>
class X<std::vector<T>, T&>
{
};

//template<typename T>
//class X<std::vector<T>, int&>
//{
//};

template<>
class X<std::vector<double>, int&>
{
};

template<typename T1, typename T2, typename T3>
class X<std::map<T1, T2>, T3&>
{
};

// 最后，还有一种超级牛X的，在tr1里面用以实现function：
template<typename T>
class Y;
//这是在声明一个类模板，既然声明了，以后就得按这个规矩来，在我们之前的编程经验里，可以重复声明一个东西没问题，但是为同一个东东重复声明出不 同的东西就不可以了，因此你就不能再声明诸如template<typename T1, typename T2> class Y;这样的声明了；其实没有什么是不能声明的，既然我们可以声明变量，声明函数，声明类，那么当然我们也可以声明函数模板或者类模板的。
template<typename R, typename P1, typename P2>
class Y<R(P1, P2)>
{
};
//针对带两个参数，有返回值的函数类型特化，这里R (P1,P2)是定义了一种类型，该类型是一个隐式的函数指针，返回R，参数为P1和P2，这种对函数指针的定义完全等同于R (*)(P1,P2)，但是前一种定义很不常见，大家一般是不会注意到这个地方的。

//函数模板的特化只能是全特化，而不能是偏特化，因此对于函数的特化就比较简单了，就是重新搞一遍就可以了，举几个例子如下：
template<class T>
T mymax(const T t1, const T t2)
{
    return t1 < t2 ? t2 : t1;
}

template<>
const char* mymax(const char* t1, const char* t2)
{
    return (strcmp(t1, t2) < 0) ? t2 : t1;
}

//但是你不能这么搞：
//template<>
//bool mymax(const char* t1, const char* t2)
//{
//    return (strcmp(t1, t2) < 0);
//}
//其实对于mymax这个模板函数的定义而言，是用一个模板参数控制了三个地方，那么你在特化的时候，就也需要用一个特定的类型修改那三处相应的地方，如果你非要返回bool，那么你只能再定义一个函数模板了：

//类模板：
//
//	* 如果类模板中含有静态成员，那么用来实例化的每种类型，都会实例化这些静态成员。
//
//	* 两个靠在一起的模板尖括号（ > ) 之间需要留个空格，否则，编译器将会认为是在使用operator>>，导致语法错误。
//
//	* 特化的实现可以和基本类模板的实现完全不同。
//
//	* 类模板可以为模板参数定义缺省值，称为缺省模板实参，并且他们还可以引用之前的模板参数。
//
//	* 成员函数模版不能被声明为虚函数。
//
//	* 类模板不能和另外一个实体共享一个名称。
//
//
//非类型模板参数：
//
//	在编译期或链接期可以确定的常值。这种参数的类型必须是下面的一种：
//
//		a> 整型或枚举
//
//		b> 指针类型( 普通对象的指针，函数指针，成员指针 )
//
//		c> 引用类型( 指向对象或者指向函数的引用 )
//
//	其他的类型目前都不允许作为非类型模板参数使用。示例：
template<typename T, int LEN>
struct stack
{
};

template<int margin>
int add(int x)
{
    return x + margin;
}

//比较混乱的例子：
template<typename Type>
void f(Type t)
{
} //f1

template<>
void f(int t)
{
} //f2

void f(int t)
{
} //f3

void f(char t)
{
} //f4

//f(3); //invoke f3
//f('3'); //invoke f4

/**
 caveat: f3 must be put after f2, or an error occurs: specialization of void f(T) [with T = int] after instantiation;
 notes: the compiler will use f3 as the instantiation for f1, and use f2 as the specialization for f1;
 rule: specialization must be before instantiation (*);

 Above we have discuss the template function, and then we'll focus on member template function.

 acronym: MTF(member template function);
 Firstly, you should pay attention to the rule: the specialization of MTF must be the outside of the class, i.e., inline should not be allowed.

 Secondly, specialization and instantiation still follow the rule (*). But you'd better put the instantiation outside of the class since specialization must be the outside of the class(the root cause is: if you wanna specialize a MTF, you should give compiler the defenition of the templcate firstly. But if you use the specialization as the inline method, the specialization will be anxious since you can put the defination of MTF outside of the class. As you know, the inline member function precedes the non-inline member function. So the compiler will chose the safest way to solve it, i.e., the specialization must be put outside of class declaration).
 */



// 一些比较难理解的元模板，注意模板是在编译时计算的。

using namespace std;
//实现1..N的递加
template<int N>
class my
{
public:
    enum
    {
        fare = my<N - 1>::fare + N
    };
};
template<>
class my<0>
{
public:
    enum
    {
        fare = 0
    };
};
//实现power(N,M)
template<int N, int M>
class mypower
{
public:
    enum
    {
        fare = N * mypower<N, M - 1>::fare
    };
};
template<int N> //半特化
class mypower<N, 0>
{
public:
    enum
    {
        fare = 1
    };
};

//实现菲波那契数组的递归
template<int N>
class fib
{
public:
    const static long long fare = fib<N - 1>::fare + fib<N - 2>::fare;
};
template<>
class fib<1>
{
public:
    const static long long fare = 1;
};
template<>
class fib<0>
{
public:
    const static long long fare = 1;
};
//N! ＝ N* （N-1）* …… *2 *1
template<int N>
class N_plus_to_1
{
public:
    const static long long fare = N * N_plus_to_1<N - 1>::fare;
};
template<>
class N_plus_to_1<1>
{
public:
    const static long long fare = 1;
};
//再来一个比较难的，模板有int，也有类
template<int N>
class Line
{
public:
    const static long long fare = N;
};
template<int N>
class Square
{
public:
    const static long long fare = N * N;
};
template<int N>
class Cube
{
public:
    const static long long fare = N * N * N;
};
template<int N, template<int N> class T>
class My_define
{
public:
/*
完全特化，此时T只是一个适合传入的类名，而不用也不可以写成T<N-1>形式
*/
    const static long long fare = My_define<N - 1, T>::fare + T<N>::fare;
};
template<template<int> class T> //依旧是半特化，或者说是局部特化
class My_define<0, T>
{
public:
    const static long long fare = 0 + T<0>::fare;
};

static void Test001()
{
    cout << my<10>::fare << endl;
    cout << mypower<2, 10>::fare << endl;
    cout << fib<90>::fare << endl; //本来用100的，结果发生溢出，还是用90吧。
    cout << N_plus_to_1<20>::fare << endl; //21就溢出了哦
    cout << My_define<2, Line>::fare << endl;
    cout << My_define<2, Square>::fare << endl;
    cout << My_define<2, Cube>::fare << endl;
}

ADD_TEST(TemplateTest, Test001);

namespace meta_template_test
{
// 模板
template<class T>
class MyType
{
public:
    int min()
    {
        return 0;
    }

    int max()
    {
        return 9;
    }
};

template<class T>
class MyClass
{
public:
    typedef MyType<T> TYPE;

public:
    TYPE get(); // 返回一个TYPE类型的对象
};

template<class T>
typename MyClass<T>::TYPE MyClass<T>::get() // 注意这个typename
{
    return MyClass<T>::TYPE();
}

void demo1()
{
    MyClass<int> x;
    MyType<int> y = x.get();

    printf("type min: %d, type max: %d\n", y.min(), y.max());
}

// 模板元（递归）
/*
 The problem: create a typedef for a signed integral type that is at least nbits in size_.
 The C++ Way
 This example is simplified and adapted from one written by Dr. Carlo Pescio in Template Metaprogramming: Make parameterized integers portable with this novel technique.
 There is no way in C++ to do conditional compilation based on the result of an expression based on template parameters, so all control flow follows from pattern matching of the template argument against various explicit template specializations. Even worse, there is no way to do template specializations based on relationships like "less than or equal to", so the example uses a clever technique where the template is recursively expanded, incrementing the template value argument by one each time, until a specialization matches. If there is no match, the result is an unhelpful recursive compiler stack overflow or internal error, or at best a strange syntax error.

 A preprocessor macro is also needed to make up for the lack of template typedefs.
 */

#include <limits.h>

template<int nbits>
struct Integer
{
    typedef typename Integer<nbits + 1>::int_type int_type;
};

template<>
struct Integer<8>
{
    typedef signed char int_type;
};

template<>
struct Integer<16>
{
    typedef short int_type;
};

template<>
struct Integer<32>
{
    typedef int int_type;
};

template<>
struct Integer<64>
{
    typedef long long int_type;
};

// If the required size_ is not supported, the metaprogram
// will increase the counter until an internal error is
// signaled, or INT_MAX is reached. The INT_MAX
// specialization does not define a int_type, so a
// compiling error is always generated
template<>
struct Integer<INT_MAX>
{
};

// A bit of syntactic sugar
#define Integer(nbits) Integer<nbits>::int_type


void demo2()
{
    Integer(8) i;
    Integer(16) j;
    Integer(29) k;
    Integer(64) l;
    Integer(20) m;
    printf("%lu %lu %lu %lu %lu\n",
           sizeof(i), sizeof(j), sizeof(k), sizeof(l), sizeof(m));
}


using namespace std;
// 1..N的累加
template<int N>
class MyAdd
{
public:
    enum
    {
        fare = MyAdd<N - 1>::fare + N
    };
};

template<>
class MyAdd<0>
{
public:
    enum
    {
        fare = 0
    };
};

// power(N, M)：N的M次方
template<int N, int M>
class MyPower
{
public:
    enum
    {
        fare = N * MyPower<N, M - 1>::fare
    };
};
template<int N> // 偏特化
class MyPower<N, 0>
{
public:
    enum
    {
        fare = 1
    };
};

// 菲波那契数列
template<int N>
struct Fib
{
    const static long long fare = Fib<N - 1>::fare + Fib<N - 2>::fare;
};

template<>
struct Fib<1>
{
    const static long long fare = 1;
};

template<>
struct Fib<0>
{
    const static long long fare = 1;
};

// 阶乘 N! ＝ N * (N-1) * ... * 2 * 1
template<int N>
class Factorial
{
public:
    const static long long fare = N * Factorial<N - 1>::fare;
};

template<>
class Factorial<1>
{
public:
    const static long long fare = 1;
};

// 一个比较难的，模板有int，也有类
template<int N>
class Line
{
public:
    const static long long fare = N;
};

template<int N>
class Square
{
public:
    const static long long fare = N * N;
};

template<int N>
class Cube
{
public:
    const static long long fare = N * N * N;
};

// 第一个参数是整形变量，第二个参数是模板类
template<int N, template<int N> class T>
class MyDefine
{
public:
    const static long long fare = MyDefine<N - 1, T>::fare + T<N>::fare;
};

template<template<int> class T>
class MyDefine<0, T> // 偏特化
{
public:
    const static long long fare = 0 + T<0>::fare;
};

void demo3()
{
    cout << MyAdd<10>::fare << endl;
    cout << MyPower<2, 10>::fare << endl;
    cout << Fib<91>::fare << endl; // long long类型在92时就会溢出，可以设为100，编译的时候会报错，这一点可以印证模板递归计算确实是发生在编译时期
    cout << Factorial<20>::fare << endl; // long long类型在21时就溢出了哦
    cout << MyDefine<2, Line>::fare << endl;
    cout << MyDefine<2, Square>::fare << endl;
    cout << MyDefine<2, Cube>::fare << endl;
}

/*
 Type Traits
 在STL中为了提供通用的操作而又不损失效率，我们用到了一种特殊的技巧，叫traits编程技巧。具体的来说，traits就是通过定义一些结构体或类，并利用模板类特化和偏特化的能力，给类型赋予一些特性，这些特性根据类型的不同而异。在程序设计中可以使用这些traits来判断一个类型的一些特性，引发C++的函数重载机制，实现同一种操作因类型不同而异的效果。traits的编程技巧极度弥补了C++语言的不足 。
 Type traits are another term for being able to Find out properties of a type at compile time.
 The C++ Way
 The following template determines if the template's argument type is a function:

 This template relies on the SFINAE (Substitution Failure Is Not An Error) principle. Why it works is a fairly advanced template topic.
 数组元素的类型不能为void类型、引用类型或函数类型。因此可构造一个测试用的成员函数模板，函数形参为模板参数的数组类型指针，这样数组元素就不能接受函数类型。同时提供该成员另一个重载版本，以应付T是函数类型的情况(SFINAE原则)，对有干扰的void类型和引用类型提供特化以表示它们不是函数类型，这样就可以辨别T了。
 代码中的U (*)[1]是一个指向数组U[1]的指针，U是数组元素的类型。可见这里的U不能为函数、引用及void类型。当U为函数、引用及void以外的其他类型时，sizeof就会使用这个版本的test函数，返回的Two的字节数2，则Yes为0，表示非函数类型。当为引用或void类型时，会使用特化版本，设置Yes为0。当为函数类型时，sizeof中的Test匹配有数组指针的那个Test时不会成功，但它能匹配另一个返回One为1个字节的test版本，根据SFINAE原则，它会使用这个版本的test，Yes设置为1。注意这里test只需声明，无需定义，因为sizeof并不会真正调用并执行该函数，不需要函数代码定义，它只是计算返回类型的字节数。
 当然，我们还可以用其他的独特构造来实现，比如只有对函数类型，F&（指向函数类型的引用）才能转化为F*。通过判断能否把一个F&转化为F*，也可辨别出F是否是函数类型。
 */
template<typename T>
class IsFunctionT
{
private:
    typedef char One;
    typedef struct
    {
        char a[2];
    } Two;
    template<typename U>
    static One test(...); // U为函数时使用这个，只需声明，无需定义
    template<typename U>
    static Two test(U (*)[1]); // U为非函数、非引用及非void类型时使用这个
public:
    enum
    {
        Yes = sizeof(test<T>(0)) == 1
    }; // 记录测试的结果
    enum
    {
        No = !Yes
    };
};

template<typename T>
class IsFunctionT<T&>
{ // T是引用类型时会使用这个局部特化，表示它不是函数类型
public:
    enum
    {
        Yes = 0
    };
    enum
    {
        No = !Yes
    };
};

template<>
class IsFunctionT<void>
{ // T是void类型时会使用这个全局特化，表示它不是函数类型
public:
    enum
    {
        Yes = 0
    };
    enum
    {
        No = !Yes
    };
};

template<>
class IsFunctionT<void const>
{ // T是void const类型时会使用这个全局特化
public:
    enum
    {
        Yes = 0
    };
    enum
    {
        No = !Yes
    };
};

// 对于void volatile和void const volatile类型也是一样
// ...

void demo4()
{
    typedef int (fp)(int);

    if (1 == IsFunctionT<fp>::Yes)
    {
        printf("fp is function");
    }
}

void Test001()
{
    demo1();
    demo2();
    demo3();
    demo4();
}

ADD_TEST(MetaTemplateTest, Test001);

} /* namespace meta_template_test */
