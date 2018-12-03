#include "curl_sync_client_test.h"
#include "curl_sync_client.h"

CurlSyncClientTest::CurlSyncClientTest()
{
}

CurlSyncClientTest::~CurlSyncClientTest()
{
}

void CurlSyncClientTest::SetUp()
{
    ASSERT_EQ(0, CurlGlobalInitialize());
}

void CurlSyncClientTest::TearDown()
{
    CurlGlobalFinalize();
}

void CurlSyncClientTest::HttpGetTest001()
{
    CurlSyncClient client;
    ASSERT_EQ(0, client.Initialize());

    CurlSyncClient::Chunk rsp;
    const char url[] = "account.moon.com/java/sso/token";
    const char data[] = "app_id=gamebox_api&timestamp=1471943327000&token=uc:sso:token:9fa47d6fe09e8c336c4ef38b5fc9a9e2&uid=54105&sign=ff8bd5efceb8785a6b1cf77d1abccb15";

    CURLcode ret = client.DoHttpGet(rsp, url, data);
    if (ret != CURLE_OK)
    {
        FAIL() << client.GetErrMsg(ret);
    }

    std::cout << rsp.buf << ", " << rsp.size << std::endl;
    rsp.Release();

    client.Finalize();
}

void CurlSyncClientTest::HttpPostTest001()
{
    CurlSyncClient client;
    ASSERT_EQ(0, client.Initialize());

    CurlSyncClient::Chunk rsp;
    const char url[] = "account.moon.com/java/sso/token";
    const char data[] = "app_id=gamebox_api&timestamp=1471943327000&token=uc:sso:token:9fa47d6fe09e8c336c4ef38b5fc9a9e2&uid=54105&sign=ff8bd5efceb8785a6b1cf77d1abccb15";

    CURLcode ret = client.DoHttpPost(rsp, url, data, strlen(data));
    if (ret != CURLE_OK)
    {
        FAIL() << client.GetErrMsg(ret);
    }

    std::cout << rsp.buf << ", " << rsp.size << std::endl;
    rsp.Release();

    client.Finalize();
}

ADD_TEST_F(CurlSyncClientTest, HttpGetTest001);
ADD_TEST_F(CurlSyncClientTest, HttpPostTest001);
