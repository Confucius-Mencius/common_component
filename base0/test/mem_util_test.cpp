#include "test_util.h"

//   operator new 和 operator delete 函数有两个重载版本，每个版本支持相关的new表达式和delete表达式：
//
//   void* operator new (size_t);        // allocate an object
//   void* operator new [] (size_t);     // allocate an array
//
//   void operator delete (void*);       // free an oject
//   void operator delete [] (void*);    // free an array
//

namespace delete_test
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
        printf("%d\n", *((int*) ((char*) p - 4))); // 前4个字节的内容
        printf("%ld\n", *((int64_t*) ((char*) p - 8))); // 在64位系统中，前8个字节的内容也不为100，不是数组中元素的个数

        printf("%p\n", p);
        delete p; // 或者delete[] p;亦可，不会有内存泄露
        p = NULL;
    }
}

class T
{
public:
    T()
    {
    }

    ~T()
    {
        printf("destructor\n");
    }

private:
    char c_;
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
        printf("%d\n", *((int*) ((char*) p - 4))); // 前4个字节的内容
        printf("%ld\n", *((int64_t*) ((char*) p - 8))); // 在64位系统中，前8个字节的内容为100，是数组中元素的个数

        printf("%p\n", p);
        delete[] p; // 换做delete p;直接段错误。在windows上未验证，可能不会段错误，但是只会调用p[0]的析构函数
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

ADD_TEST(MemUtilTest, Test001);
ADD_TEST(MemUtilTest, Test002);
ADD_TEST(MemUtilTest, Test003);
} // namespace delete_test
