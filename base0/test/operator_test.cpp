#include "test_util.h"

//赋值运算符和拷贝构造函数的调用选择：究竟什么时候编译器会调用拷贝构造函数，什么时候调用赋值运算符呢？
//注意：默认的、标准的、自动生成的赋值运算符为：T& operator=(const T&)，拷贝构造函数为：T(const T&)

namespace operator_test
{
class Test1
{
public:
    Test1()
    {
        i_ = 0;
        std::cout << "Test1()" << std::endl;
    }

    Test1(int i)
    {
        i_ = i;
        std::cout << "Test1(int i)" << std::endl;
    }

    Test1(const Test1& rhs)
    {
        i_ = rhs.i_;
        std::cout << "Test1(const Test1& rhs)" << std::endl;
    }

    Test1& operator=(const Test1& rhs)
    {
        std::cout << "operator =(const Test1& rhs)" << std::endl;

        if (&rhs != this)
        {
            i_ = rhs.i_;
        }

        return *this;
    }

    Test1& operator=(int i)
    {
        std::cout << "operator =(int i)" << std::endl;
        i_ = i;
        return *this;
    }

    void Print()
    {
        std::cout << "i= " << i_ << std::endl;
    }

private:
    int i_;
};

void Test001()
{
    Test1 t0;
    t0.Print();

    Test1 t1(1);
    t1.Print();

    Test1 t2(t1);
    t2.Print();

    Test1 t3 = t2;
    t3.Print();

    Test1 t4 = 4;
    t4.Print();
}

class Test2
{
public:
    Test2()
    {
        i_ = 0;
        std::cout << "Test2()" << std::endl;
    }

    explicit Test2(int i)
    {
        i_ = i;
        std::cout << "Test2(int i)" << std::endl;
    }

    Test2(const Test2& rhs)
    {
        i_ = rhs.i_;
        std::cout << "Test2(const Test2& rhs)" << std::endl;
    }

    Test2& operator=(const Test2& rhs)
    {
        std::cout << "operator =(const Test2& rhs)" << std::endl;

        if (&rhs != this)
        {
            i_ = rhs.i_;
        }

        return *this;
    }

    Test2& operator=(int i)
    {
        std::cout << "operator =(int i)" << std::endl;
        i_ = i;
        return *this;
    }

    void Print()
    {
        std::cout << "i= " << i_ << std::endl;
    }

private:
    int i_;
};

// explicit
void Test002()
{
    Test2 t0;
    t0.Print();

    Test2 t1(1);
    t1.Print();

    Test2 t2(t1);
    t2.Print();

    Test2 t3 = t2;
    t3.Print();

    //Test2 t4 = 4; // 编译不过
    //t4.Print();

    // 下面的才行
    Test2 t5;
    t5 = 4;
    t5.Print();
}

class Test3
{
public:
    Test3()
    {
        i_ = 0;

    }

    Test3& operator=(const Test3& rhs)
    {
        std::cout << "operator =(const Test3& rhs)" << std::endl;

        if (&rhs != this)
        {
            i_ = rhs.i_;
        }

        return *this;
    }

    Test3& operator=(int i)
    {
        std::cout << "operator =(int i)" << std::endl;
        i_ = i;
        return *this;
    }

    void Print()
    {
        std::cout << "i= " << i_ << std::endl;
    }

private:
    int i_;
};

void Test003()
{
    Test3 t0;
    t0.Print();

    //Test3 t1(1); // 编译不过
    //t1.Print();

    Test3 t2(t0);
    t2.Print();

    Test3 t3 = t2;
    t3.Print();

//    Test3 t4 = 4; // 编译不过
//    t4.Print();

    // 下面的才行
    Test3 t5;
    t5 = 5;
    t5.Print();
}

class Test4
{
public:
    Test4()
    {
        i_ = 0;
    }

    void Print()
    {
        std::cout << "i= " << i_ << std::endl;
    }

private:
    int i_;
};

void Test004()
{
    Test4 t0;
    t0.Print();

//    Test4 t1(1); // 编译不过
//    t1.Print();

    Test4 t2(t0);
    t2.Print();

    Test4 t3 = t2;
    t3.Print();

    //Test4 t4 = 4; // 编译不过
    //t4.Print();
}

ADD_TEST(OperatorTest, Test001);
ADD_TEST(OperatorTest, Test002);
ADD_TEST(OperatorTest, Test003);
ADD_TEST(OperatorTest, Test004);
} // namespace operator_test
