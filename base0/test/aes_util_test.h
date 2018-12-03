#ifndef BASE_TEST_AES_UTIL_TEST_H_
#define BASE_TEST_AES_UTIL_TEST_H_

#include "test_util.h"

class AESUtilTest : public GTest
{
public:
    AESUtilTest();
    virtual ~AESUtilTest();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
};

#endif // BASE_TEST_AES_UTIL_TEST_H_
