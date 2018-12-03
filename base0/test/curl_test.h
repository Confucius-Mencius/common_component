#ifndef BASE_TEST_CURL_TEST_H_
#define BASE_TEST_CURL_TEST_H_

#include <curl/curl.h>
#include "test_util.h"

class CurlTest : public GTest
{
public:
    CurlTest();
    virtual ~CurlTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test_EasyGet();
    void Test_EasyPost();
    void Test_MultiGet();
    void Test_MultiPost();
    void Test_ShareGet();
    void Test_SharePost();
    void Test_ConnectTest();

private:
    bool CanConnect(const char* addr_port);

private:
};

#endif // BASE_TEST_CURL_TEST_H_
