#ifndef BASE_TEST_DEMO_TEST_H_
#define BASE_TEST_DEMO_TEST_H_

#include "test_util.h"

namespace demo_test
{
class DemoTest : public GTest
{
public:
    static void SetUpTestCase();
    static void TearDownTestCase();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
};
}

#endif // BASE_TEST_DEMO_TEST_H_
