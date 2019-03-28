#include "test_util.h"

namespace move_test
{
//左值就是有名字的变量（对象），可以被赋值，可以在多条语句中使用，而右值呢，就是临时变量（对象），没有名字，只能在一条语句中出现，不能被赋值。
//左值的声明符号为”&”， 为了和左值区分，右值的声明符号为”&&”。
//声明为右值引用的变量可以是左值或右值。区别标准是：如果它有一个名字，那么它就是一个左值；否则，它是一个右值。

//右值引用的意义
//直观意义：为临时变量续命，也就是为右值续命，因为右值在表达式结束后就消亡了，如果想继续使用右值，那就会动用昂贵的拷贝构造函数。（关于这部分，推荐一本书《深入理解C++11》）
//右值引用是用来支持转移语义的。转移语义可以将资源 ( 堆，系统对象等 ) 从一个对象转移到另一个对象，这样能够减少不必要的临时对象的创建、拷贝以及销毁，
// 能够大幅度提高 C++ 应用程序的性能。临时对象的维护 ( 创建和销毁 ) 对性能有严重影响。
//转移语义是和拷贝语义相对的，可以类比文件的剪切与拷贝，当我们将文件从一个目录拷贝到另一个目录时，速度比剪切慢很多。
//通过转移语义，临时对象中的资源能够转移到其它的对象里。
//在现有的 C++ 机制中，我们可以定义拷贝构造函数和赋值运算符。要实现转移语义，则需要定义转移构造函数和转移赋值运算符。
// 对于右值的拷贝和赋值会调用转移构造函数和转移赋值运算符。如果转移构造函数和转移拷贝运算符没有定义，那么就遵循现有的机制，拷贝构造函数和赋值运算符会被调用。
//普通的函数和操作符也可以利用右值引用操作符实现转移语义。

//std::move
//    c++11中提供了std::move()来将左值转换为右值，从而方便的使用移动语义。move是将对象的状态或者所有权从一个对象转移到另一个对象，只是转移，没有内存拷贝。
//    c++中所有容器都实现了move语义，方便我们实现性能优化。move只是转移了资源的控制权，本质上是将左值强制转换为右值引用，以用于move语义，避免含有资源的对象发生无谓的拷贝。
//    move对于拥有形如对内存、文件句柄等资源的成员的对象有效。如果是一些基本类型，比如int或char[10]数组等，如果使用move，仍然会发生拷贝（因为没有对应的移动构造函数），所以move对于含资源的对象来说更有意义。
//
//std::move无条件地把参数转换为右值，而std::forward在满足条件下才会执行std::move的转换。
//参考：https://blog.csdn.net/coolmeme/article/details/44459999

void process_value(int& i)
{
    std::cout << "LValue processed: " << i << std::endl;
}

void process_value(int&& i)
{
    std::cout << "RValue processed: " << i << std::endl;
}

void Test001()
{
    int a = 0;

    process_value(a);

//    Process_value 函数被重载，分别接受左值和右值。由输出结果可以看出，临时对象是作为右值处理的。
    process_value(1);

    process_value(a);

//    x 是一个右值引用，指向一个右值3，但是由于x是有名字的，所以x在这里被视为一个左值
    int&& x = 2;
    process_value(x);
}

class MyString
{
public:
    MyString()
    {
        data_ = nullptr;
        len_ = 0;
    }

    virtual ~MyString()
    {
        if (data_ != nullptr)
        {
            free(data_);
            data_ = nullptr;
        }
    }

    MyString(const char* p)
    {
        len_ = strlen(p);
        InitData(p);
        std::cout << "Constructor is called! source: " << p << std::endl;
    }

    MyString(const MyString& str)
    {
        len_ = str.len_;
        InitData(str.data_);
        std::cout << "Copy Constructor is called! source: " << str.data_ << std::endl;
    }

    MyString& operator=(const MyString& str)
    {
        if (this != &str)
        {
            if (data_ != nullptr)
            {
                delete[] data_;
                data_ = nullptr;
                len_ = 0;
            }

            len_ = str.len_;
            InitData(str.data_);
        }

        std::cout << "Copy Assignment is called! source: " << str.data_ << std::endl;
        return *this;
    }

//    有下面几点需要对照代码注意：
//    1. 参数（右值）的符号必须是右值引用符号，即“&&”。
//    2. 参数（右值）不可以是常量，因为我们需要修改右值。
//    3. 参数（右值）的资源链接和标记必须修改。否则，右值的析构函数就会释放资源。转移到新对象的资源也就无效了。
    MyString(MyString&& str)
    {
        len_ = str.len_;
        data_ = str.data_;
        str.len_ = 0;
        str.data_ = nullptr;

        std::cout << "Move Constructor is called! source: " << data_ << std::endl;
    }

    MyString& operator=(MyString&& str)
    {
        if (this != &str)
        {
            if (data_ != nullptr)
            {
                delete[] data_;
                data_ = nullptr;
                len_ = 0;
            }

            len_ = str.len_;
            data_ = str.data_;
            str.len_ = 0;
            str.data_ = nullptr;
        }

        std::cout << "Move Assignment is called! source: " << data_ << std::endl;
        return *this;
    }

private:
    void InitData(const char* s)
    {
        data_ = new char[len_ + 1];
        memcpy(data_, s, len_);
        data_[len_] = '\0';
    }

private:
    char* data_;
    size_t len_;
};

void Test002()
{
//    MyString(“Hello”) 和 MyString(“World”) 都是临时对象，也就是右值
    MyString my_str;
    my_str = MyString("Hello");

    std::vector<MyString> vec;
    vec.push_back(MyString("World"));

//    没有转移构造函数和转移赋值运算符的运行结果：
//    Constructor is called! source: Hello
//    Copy Assignment is called! source: Hello
//    Constructor is called! source: World
//    Copy Constructor is called! source: World

//    增加了转移构造函数和转移赋值运算符的运行结果：
//    Constructor is called! source: Hello
//    Move Assignment is called! source: Hello
//    Constructor is called! source: World
//    Move Constructor is called! source: World
}

void TestSTLObject()
{
    std::string str = "Hello";
    std::vector<std::string> v;

    // uses the push_back(const T&) overload, which means
    // we'll incur the cost of copying str
    v.push_back(str);
    std::cout << "After copy, str is \"" << str << "\"\n";

    // uses the rvalue reference push_back(T&&) overload,
    // which means no strings will be copied; instead, the contents
    // of str will be moved into the vector.  This is less
    // expensive, but also means str might now be empty.
    v.push_back(std::move(str));
    std::cout << "After move, str is \"" << str << "\"\n";

    std::cout << "The contents of the vector are \"" << v[0]
        << "\", \"" << v[1] << "\"\n";

}

ADD_TEST(MoveTest, Test001);
ADD_TEST(MoveTest, Test002);
ADD_TEST(MoveTest, TestSTLObject);
}
