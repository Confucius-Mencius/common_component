#ifndef BASE_TEST_ZK_UTIL_TEST_H_
#define BASE_TEST_ZK_UTIL_TEST_H_

#include "test_util.h"

class ZkUtilTest : public GTest
{
public:
    ZkUtilTest();
    virtual ~ZkUtilTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001(); // init close
};

#endif // BASE_TEST_ZK_UTIL_TEST_H_
