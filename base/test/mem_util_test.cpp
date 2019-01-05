#include "mem_util.h"
#include "test_util.h"
#include "simple_log.h"

//   operator new 和 operator delete 函数有两个重载版本，每个版本支持相关的new表达式和delete表达式：
//
//   void* operator new (size_t);        // allocate an object
//   void* operator new [] (size_t);     // allocate an array
//
//   void operator delete (void*);       // free an oject
//   void operator delete [] (void*);    // free an array
//

namespace mem_util_test
{
/**
 * @brief 基本类型new[] delete[]测试
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention
 * 1，基本类型没有析构函数，释放内存时不需要调用析构函数，故不需要记录数组的元素个数，delete和delete[]效果相同
  */
void Test001()
{
    int* p = NULL;

    for (int i = 0; i < 10; ++i)
    {
        p = new int[100];
        LOG_C("%d", *((int*) ((char*) p - 4))); // 前4个字节的内容
        LOG_C("%ld", *((int64_t*) ((char*) p - 8))); // 在64位系统中，前8个字节的内容也不为100，不是数组中元素的个数

        LOG_C("%p", p);
        delete p; // 或者delete[] p;亦可，不会有内存泄露
        p = NULL;
    }
}

class T
{
public:
    explicit T(char c = 'x')
    {
        c_ = c;
        u_ = 0xffffffff;
    }

    ~T()
    {
        LOG_C("destructor");
    }

    void Set(char c)
    {
        c_ = c;
    }

    char Get() const
    {
        return c_;
    }

private:
    char c_;
    unsigned int u_;
};

/**
 * @brief 自定义类型new[] delete[]测试
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void Test002()
{
    T* p = NULL;

    for (int i = 0; i < 10; ++i)
    {
        p = new T[100];
        LOG_C("%d", *((int*) ((char*) p - 4))); // 前4个字节的内容
        LOG_C("%ld", *((int64_t*) ((char*) p - 8))); // 在64位系统中，前8个字节的内容为100，是数组中元素的个数

        LOG_C("%p", p);
        delete[] p; // 换做delete p;直接段错误。在windows上未验证，可能不会段错误，可能只会调用p[0]的析构函数
        p = NULL;
    }
}

// 三指针测试
void Func(char*** p, int& n)
{
    char** p1 = new char* [10];

    for (int i = 0; i < 10; ++i)
    {
        p1[i] = new char[24];
    }

    n = 10;
    *p = p1;
}

void Release(char*** p, int n)
{
    char** p1 = *p;
    for (int i = 0; i < n; ++i)
    {
        delete[] p1[i];
    }

    delete[] p1;
    *p = NULL;
}

void Test003()
{
    char** p = NULL;
    int n;

    Func(&p, n);
    Release(&p, n);
}

// placement new分配和释放单个对象
void Test004()
{
    unsigned char buf[256];
    T* p = Construct((T*) buf); // 分配对象
    EXPECT_EQ((int) p->Get(), (int) 'x');
    Destory(p); // 释放对象
}

void Test005()
{
    T t;
    t.Set('y');

    unsigned char buf[256];
    T* p = Construct((T*) buf, t); // 分配对象
    EXPECT_EQ((int) p->Get(), (int) 'y');
    Destory(p); // 释放对象
}

// placement new分配和释放一批连续的对象
void Test006()
{
    unsigned char buf[256];
    T* p = Construct((T*) buf, 10); // 分配对象
    EXPECT_EQ((int) p->Get(), (int) 'x');
    Destory(p, 10); // 释放对象
}

void Test007()
{
    T t;
    t.Set('y');

    unsigned char buf[256];
    T* p = Construct((T*) buf, t, 10); // 分配对象
    EXPECT_EQ((int) p->Get(), (int) 'y');
    Destory(p, 10); // 释放对象
}

ADD_TEST(MemUtilTest, Test001);
ADD_TEST(MemUtilTest, Test002);
ADD_TEST(MemUtilTest, Test003);
ADD_TEST(MemUtilTest, Test004);
ADD_TEST(MemUtilTest, Test005);
ADD_TEST(MemUtilTest, Test006);
ADD_TEST(MemUtilTest, Test007);
} // namespace mem_util_test
