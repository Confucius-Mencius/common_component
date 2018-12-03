#ifndef BASE_TEST_LIBCURL_TEST_H_
#define BASE_TEST_LIBCURL_TEST_H_

#include "test_util.h"

class LibcurlTest : public GTest
{
public:
    LibcurlTest();
    virtual ~LibcurlTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
    void Test005();
    void Test006();
    void Test007();
    void Test007_001();
    void Test007_002();
    void Test008();
    void Test009();
    void Test010();
    void Test011();
    void Test011_001();
    void Test012();
    void Test012_001();
};

#endif // BASE_TEST_LIBCURL_TEST_H_
