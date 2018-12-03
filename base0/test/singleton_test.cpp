#include "test_util.h"
#include "singleton.h"

namespace singleton_test
{
class Demo
{
public:
    void SayHello()
    {
        std::cout << "hello, world!" << std::endl;
    }
};

#define DemoS Singleton<Demo>::Instance()

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
    Demo* p1 = DemoS;
    EXPECT_TRUE(p1 != NULL);
    Demo* p2 = DemoS;
    EXPECT_TRUE(p2 == p1);
}

ADD_TEST(SingletonTest, Test001);
} /* namespace singleton_test */
