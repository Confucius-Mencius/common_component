#ifndef BASE_TEST_RSA_UTIL_TEST_H_
#define BASE_TEST_RSA_UTIL_TEST_H_

#include "test_util.h"
#include "rsa_util.h"

class RSAUtilTest : public GTest
{
public:
    RSAUtilTest();
    virtual ~RSAUtilTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
    void Test005();
    void Test006();

    void Test007();
    void Test008();
    void Test009();
    void Test010();
    void Test011();
    void Test012();
    void Test013();
    void Test014();
    void Test015();
    void Test016();

private:
    RSAUtil rsa_util_;
};

#endif // BASE_TEST_RSA_UTIL_TEST_H_
