#ifndef BASE_TEST_CURL_SYNC_CLIENT_TEST_H_
#define BASE_TEST_CURL_SYNC_CLIENT_TEST_H_

#include "test_util.h"

class CurlSyncClientTest : public GTest
{
public:
    CurlSyncClientTest();
    virtual ~CurlSyncClientTest();

    virtual void SetUp();
    virtual void TearDown();

    void HttpGetTest001();
    void HttpPostTest001();

private:

};

#endif // BASE_TEST_CURL_SYNC_CLIENT_TEST_H_
