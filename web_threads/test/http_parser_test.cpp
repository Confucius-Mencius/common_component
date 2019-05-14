#include "http_parser_test.h"
#include "log_util.h"

namespace tcp
{
namespace web
{
HTTPParserTest::HTTPParserTest() : http_parser_()
{
}

HTTPParserTest::~HTTPParserTest()
{
}

void HTTPParserTest::SetUp()
{
}

void HTTPParserTest::TearDown()
{
}

/**
 * @brief
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void HTTPParserTest::Test001()
{
    const char get_req[] =
        "GET /favicon.ico HTTP/1.1\r\n"
        "Host: 0.0.0.0=5000\r\n"
        "User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9) Gecko/2008061015 Firefox/3.0\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,/;q=0.8\r\n"
        "Accept-Language: en-us,en;q=0.5\r\n"
        "Accept-Encoding: gzip,deflate\r\n"
        "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
        "Keep-Alive: 300\r\n"
        "Connection: keep-alive\r\n"
        "\r\n";

    http_parser_.Execute(get_req, strlen(get_req));
}

void HTTPParserTest::Test002()
{
    const char post_req[] =
        "POST /post_identity_body_world?q=search#hey HTTP/1.1\r\n"
        "Accept: /\r\n"
        "Transfer-Encoding: identity\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "World";

    http_parser_.Execute(post_req, strlen(post_req));
}

void HTTPParserTest::Test003()
{
    const char ws_req[] =
        "GET / HTTP/1.1\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Host: example.com\r\n"
        "Origin: http://example.com\r\n"
        "Sec-WebSocket-Key: sN9cRrP/n9NdMgdcy2VJFQ==\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "\r\n";

    http_parser_.Execute(ws_req, strlen(ws_req));
}

void HTTPParserTest::Test004()
{
    const char get_req[] = "GET /crossdomain.xml HTTP/1.1\r\nHost: 127.0.0.1:10080\r\nAccept-Encoding: identity\r\n\r\n";
    http_parser_.Execute(get_req, strlen(get_req));
}

void HTTPParserTest::Test005()
{
    const char post_req[] =
        "POST / HTTP/1.1\r\n"
        "Content-Type:application/x-www-form-urlencoded\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Host: w.sohu.com\r\n"
        "Content-Length: 21\r\n"
        "Connection: Keep-Alive\r\n"
        "Cache-Control: no-cache\r\n"
        "\r\n"
        "txt1=hello&txt2=world";

    http_parser_.Execute(post_req, strlen(post_req));
}

void HTTPParserTest::Test006()
{
    const char post_req[] =
        "POST /t2/upload.do HTTP/1.1\r\n"
        "User-Agent: SOHUWapRebot\r\n"
        "Accept-Language: zh-cn,zh;q=0.5\r\n"
        "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.7\r\n"
        "Connection: keep-alive\r\n"
        "Content-Length: 60408\r\n"
        "Content-Type:multipart/form-data; boundary=ZnGpDtePMx0KrHh_G0X99Yef9r8JZsRJSXC\r\n"
        "Host: w.sohu.com\r\n"
        "\r\n"
        "--ZnGpDtePMx0KrHh_G0X99Yef9r8JZsRJSXC\r\n"
        "Content-Disposition: form-data;name=\"desc\"\r\n"
        "Content-Type: text/plain; charset=UTF-8\r\n"
        "Content-Transfer-Encoding: 8bit\r\n"
        "\r\n"
        "[......][......][......][......]...........................\r\n"
        "\r\n"
        "--ZnGpDtePMx0KrHh_G0X99Yef9r8JZsRJSXC\r\n"
        "Content-Disposition: form-data;name=\"pic\"; filename=\"photo.jpg\"\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Transfer-Encoding: binary\r\n"
        "\r\n"
        "[图片二进制数据]\r\n"
        "\r\n"
        "--ZnGpDtePMx0KrHh_G0X99Yef9r8JZsRJSXC--";

    http_parser_.Execute(post_req, strlen(post_req));
}

ADD_TEST_F(HTTPParserTest, Test001);
ADD_TEST_F(HTTPParserTest, Test002);
ADD_TEST_F(HTTPParserTest, Test003);
ADD_TEST_F(HTTPParserTest, Test004);
//ADD_TEST_F(HTTPParserTest, Test005); TODO
//ADD_TEST_F(HTTPParserTest, Test006);
}
}
