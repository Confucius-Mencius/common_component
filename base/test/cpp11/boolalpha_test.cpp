#include "test_util.h"

namespace boolalpha_test
{

void Test001()
{
    std::cout << "true is " << true << std::endl;
    std::cout << "false is " << false << std::endl;

    // 运行下面这个语句, 在输出流中的bool值将发生变化
    std::cout << std::boolalpha;
    std::cout << "true is " << true << std::endl;
    std::cout << "false is " << false << std::endl;

    // 运行下面这个语句, 在输出流中的bool值将恢复成0, 1值
    std::cout << std::noboolalpha;
    std::cout << "true is " << true << std::endl;
    std::cout << "false is " << false << std::endl;

    /* 输出如下:
     true is 1
    false is 0
    true is true
    false is false
    true is 1
    false is 0
    */
}

ADD_TEST(BoolalphaTest, Test001);
}
