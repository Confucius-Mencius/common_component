#ifndef BASE_TEST_RC4_UTIL_TEST_H_
#define BASE_TEST_RC4_UTIL_TEST_H_

#include "test_util.h"

class RC4UtilTest : public GTest
{
public:
    RC4UtilTest();
    virtual ~RC4UtilTest();

    void Test001();
    void Test002();
};

#endif // BASE_TEST_RC4_UTIL_TEST_H_
