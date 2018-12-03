#ifndef BASE_TEST_RAND_STR_TEST_H_
#define BASE_TEST_RAND_STR_TEST_H_

// 随机生成固定长度的字符串

#include "test_util.h"

class RandStrTest : public GTest
{
public:
    RandStrTest();
    virtual ~RandStrTest();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
    void Test005();
    void Test006();

private:

};

#endif // BASE_TEST_RAND_STR_TEST_H_s
