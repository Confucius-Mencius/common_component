#ifndef BASE_TEST_CURL_MULTI_TEST_H_
#define BASE_TEST_CURL_MULTI_TEST_H_

#include "test_util.h"

class CurlMultiTest : public GTest
{
public:
    CurlMultiTest();
    virtual ~CurlMultiTest();

    void Test001();

private:

};

#endif // BASE_TEST_CURL_MULTI_TEST_H_
