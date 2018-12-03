#ifndef BASE_TEST_GLOBAL_TEST_ENV_H_
#define BASE_TEST_GLOBAL_TEST_ENV_H_

#include <gmock/gmock.h>

// 全局测试环境
class GlobalTestEnv : public testing::Environment
{
public:
    GlobalTestEnv();
    virtual ~GlobalTestEnv();

    // 该函数在所有的测试用例组之前执行
    virtual void SetUp();

    // 该函数在所有的测试用例组之后执行
    virtual void TearDown();
};

extern GlobalTestEnv* g_global_test_env;

#endif // BASE_TEST_GLOBAL_TEST_ENV_H_
