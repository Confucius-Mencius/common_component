#include "curl_test.h"

//Persistent connections means that libcurl can re-use the same connection for several transfers, if the conditions are right.
//
//libcurl will always attempt to use persistent connections. Whenever you use curl_easy_perform or curl_multi_perform etc, libcurl will attempt to use an existing connection to do the transfer, and if none exists it'll open a new one that will be subject for re-use on a possible following call to curl_easy_perform or curl_multi_perform.
//
//To allow libcurl to take full advantage of persistent connections, you should do as many of your file transfers as possible using the same handle.
//
//If you use the easy interface, and you call curl_easy_cleanup, all the possibly open connections held by libcurl will be closed and forgotten.
//
//When you've created a multi handle and are using the multi interface, the connection pool is instead kept in the multi handle so closing and creating new easy handles to do transfers will not affect them. Instead all added easy handles can take advantage of the single shared pool.

//官网例子 https://curl.haxx.se/libcurl/c/example.html

CurlTest::CurlTest()
{

}

CurlTest::~CurlTest()
{

}

void CurlTest::SetUp()
{
    CURLcode ret = curl_global_init(CURL_GLOBAL_ALL);
    if (ret != CURLE_OK)
    {
        const char* err = curl_easy_strerror(ret);
        FAIL() << err;
    }
}

void CurlTest::TearDown()
{
    curl_global_cleanup();
}

void CurlTest::Test_EasyGet()
{
    CURL* curl = curl_easy_init(); // 如果需要重用这个句柄，在每次执行perform后不要cleanup，调用curl_easy_reset重置所有opt，然后设置需要的opt，再调用perform
    if (NULL == curl)
    {
        FAIL() << "failed to call curl_easy_init";
    }

    CURLcode ret = curl_easy_setopt(curl, CURLOPT_URL, "www.moon.com");
    if (ret != CURLE_OK)
    {
        const char* err = curl_easy_strerror(ret);
        FAIL() << err;
    }

    ret = curl_easy_setopt(curl, CURLOPT_HEADER, 1);
    if (ret != CURLE_OK)
    {
        const char* err = curl_easy_strerror(ret);
        FAIL() << err;
    }

    ret = curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    if (ret != CURLE_OK)
    {
        const char* err = curl_easy_strerror(ret);
        FAIL() << err;
    }

    ret = curl_easy_perform(curl);
    if (ret != CURLE_OK)
    {
        const char* err = curl_easy_strerror(ret);
        FAIL() << err;
    }

    curl_easy_cleanup(curl);
}

void CurlTest::Test_EasyPost()
{

}

void CurlTest::Test_MultiGet()
{

}

void CurlTest::Test_MultiPost()
{

}

void CurlTest::Test_ShareGet()
{

}

void CurlTest::Test_SharePost()
{

}

static int wait_on_socket(curl_socket_t sockfd, long timeout_ms)
{
    struct timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    fd_set infd, outfd, errfd;
    FD_ZERO(&errfd);
//    FD_ZERO(&infd);
    FD_ZERO(&outfd);

    FD_SET(sockfd, &errfd); /* always check for error */
//    FD_SET(sockfd, &infd);
    FD_SET(sockfd, &outfd);

    /* select() returns the number of signalled sockets or -1 */
    int ret = select(sockfd + 1, &infd, &outfd, &errfd, &tv);
    if (1 == ret && FD_ISSET(sockfd, &outfd))
    {
        return 1;
    }

    return 0;
}

bool CurlTest::CanConnect(const char* addr_port)
{
    CURL* connection_test_curl = curl_easy_init();
    if (NULL == connection_test_curl)
    {
        std::cout << "failed to call curl_easy_init" << std::endl;
        return false;
    }

    curl_easy_reset(connection_test_curl);
    curl_easy_setopt(connection_test_curl, CURLOPT_URL, addr_port);
    curl_easy_setopt(connection_test_curl, CURLOPT_CONNECT_ONLY, 1L);

    CURLcode ret = curl_easy_perform(connection_test_curl);
    if (CURLE_OK != ret)
    {
        std::cout << "failed to call curl_easy_perform: " << curl_easy_strerror(ret) << std::endl;
        return false;
    }

    long sockextr;

    ret = curl_easy_getinfo(connection_test_curl, CURLINFO_LASTSOCKET, &sockextr);
    if (CURLE_OK != ret)
    {
        std::cout << "failed to call curl_easy_getinfo: " << curl_easy_strerror(ret) << std::endl;
        return false;
    }

    curl_socket_t sockfd = sockextr;

    /* wait for the socket to become ready for sending */
    if (!wait_on_socket(sockfd, 60000L)) // 10秒
    {
        std::cout << "timeout" << std::endl;
        return false;
    }

    curl_easy_cleanup(connection_test_curl);
    return true;
}

void CurlTest::Test_ConnectTest()
{
    bool ret = CanConnect("127.0.0.1:10001");
    std::cout << ret << std::endl;

    ret = CanConnect("www.moon.com");
    std::cout << ret << std::endl;

    ret = CanConnect("www.baidu.com");
    std::cout << ret << std::endl;

    ret = CanConnect("www.google.com");
    std::cout << ret << std::endl;
}

ADD_TEST_F(CurlTest, Test_EasyGet);
ADD_TEST_F(CurlTest, Test_ConnectTest);
