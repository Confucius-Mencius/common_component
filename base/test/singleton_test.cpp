#include<thread>
#include "simple_log.h"
#include "singleton.h"
#include "test_util.h"

namespace singleton_test
{
class Demo1
{
public:
    Demo1()
    {
        LOG_CPP("Demo1");
    }

    ~Demo1()
    {
        LOG_CPP("~Demo1");
    }

    void SayHello()
    {
        LOG_CPP("hello, world!");
    }
};

#define Demo1Instance Singleton<Demo1>::Instance()

/**
 * @brief singleton测试-两次获取的是同一个对象
 * @details
 *  - Set Up:
 1，
 *  - Expectation:
 1，
 *  - Tear Down:
 1，
 * @note
 */
void Test001()
{
    Demo1Instance->SayHello();

    Demo1* p1 = Demo1Instance;
    EXPECT_TRUE(p1 != NULL);
    Demo1* p2 = Demo1Instance;
    EXPECT_TRUE(p2 == p1);
}

class Demo2
{
public:
    Demo2()
    {
        LOG_CPP("Demo2");
    }

    ~Demo2()
    {
        LOG_CPP("~Demo2");
    }

    void SayHello()
    {
        LOG_CPP("hello, world!");
    }
};

#define Demo2Instance Singleton<Demo2>::Instance()

// 多线程同时使用一个singleton测试
void show()
{
    Demo2Instance->SayHello();
}

void Test002()
{
    std::thread threads[100];

    for (int i = 0; i < 100; ++i)
    {
        threads[i] = std::thread(show);
    }

    for (int i = 0; i < 100; ++i)
    {
        threads[i].join();
    }
}

ADD_TEST(SingletonTest, Test001);
ADD_TEST(SingletonTest, Test002);
} /* namespace singleton_test */
