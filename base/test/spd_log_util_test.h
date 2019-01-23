#ifndef BASE_TEST_SPD_LOG_UTIL_TEST_H_
#define BASE_TEST_SPD_LOG_UTIL_TEST_H_

#include "test_util.h"

namespace spd_log_util_test
{
class SPDLogUtilTest : public GTest
{
public:
    SPDLogUtilTest();
    virtual ~SPDLogUtilTest();

    static void SetUpTestCase();
    static void TearDownTestCase();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
};
}

#endif // BASE_TEST_SPD_LOG_UTIL_TEST_H_
