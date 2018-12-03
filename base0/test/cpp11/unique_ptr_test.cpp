#include "unique_ptr_test.h"

//unique_ptr“唯一”拥有其所指对象，同一时刻只能有一个unique_ptr指向给定对象（通过禁止拷贝语义、只有移动语义来实现）。
//相比与原始指针unique_ptr用于其RAII的特性，使得在出现异常的情况下，动态资源能得到释放。
//unique_ptr指针本身的生命周期：从unique_ptr指针创建时开始，直到离开作用域。离开作用域时，若其指向对象，则将其所指对象销毁(默认使用delete操作符，用户可指定其他操作)。
//unique_ptr指针与其所指对象的关系：在智能指针生命周期内，可以改变智能指针所指对象，如创建智能指针时通过构造函数指定、通过reset方法重新指定(会自动释放之前管理的旧对象）、
// 通过release方法释放所有权、通过移动语义转移所有权。

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

        unique_ptr_test::Person* p = uptr2.release(); //释放智能指针的所有权，必须手工释放对象，否则内存泄露
        EXPECT_EQ(nullptr, uptr2.get());

        delete p; // 手工释放对象

        std::unique_ptr<unique_ptr_test::Person> uptr3(new unique_ptr_test::Person(3));  //绑定动态对象

        uptr3.reset(new unique_ptr_test::Person(4));    //"绑定”另一个动态对象，会自动释放之前的对象
        EXPECT_NE(nullptr, uptr3.get());
    }
    //超過uptr的作用域，內存釋放


//    由于unique_ptr有std::unique_ptr<T[]>的重载函数，所以它可以用来管理数组资源
    std::unique_ptr<int[]> pArray(new int[3]{1, 3, 3});

    std::unique_ptr<int> up;     // 创建一个空的智能指针
    EXPECT_EQ(nullptr, up.get());

    //    注意，不能将一个原始指针直接赋值给一个智能指针，如下所示，原因是一个是类，一个是指针。
//    std::unique_ptr<int> p4 = new int(1);// error

    std::unique_ptr<int> sp(new int(88));
    std::vector<std::unique_ptr<int>> vec;
    vec.push_back(std::move(sp));
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

ADD_TEST_F(UniquePtrTest, Test001);
