#ifndef BASE_TEST_OPENSSL_TEST_H_
#define BASE_TEST_OPENSSL_TEST_H_

#include "test_util.h"

class OpensslTest : public GTest
{
public:
    OpensslTest();
    virtual ~OpensslTest();

    void sha1_test();
    void Test001();

private:

};

#endif // BASE_TEST_OPENSSL_TEST_H_
