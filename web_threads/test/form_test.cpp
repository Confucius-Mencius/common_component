#include "form_test.h"
#include "log_util.h"

namespace tcp
{
namespace web
{
FormTest::FormTest() : http_parser_()
{
}

FormTest::~FormTest()
{
}

void FormTest::SetUp()
{
}

void FormTest::TearDown()
{
}

void FormTest::Test001()
{
    const char post_req[] =
        "POST / HTTP/1.1\r\n"
        "Content-Type:application/x-www-form-urlencoded\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Host: w.sohu.com\r\n"
        "Content-Length: 88\r\n"
        "Connection: Keep-Alive\r\n"
        "Cache-Control: no-cache\r\n"
        "\r\n"
        "txt1%3dhello%26txt2%3dworld%26a%3d%e4%bd%a0%e5%a5%bd%ef%bc%8c%e4%b8%96%e7%95%8c%ef%bc%81"; // "txt1=hello&txt2=world&a=你好，世界！"

    http_parser_.Execute(post_req, strlen(post_req));
}

void FormTest::Test002()
{
    const char post_req[] =
        "GET /xx?txt1%3dhello%26txt2%3dworld%26a%3d%e4%bd%a0%e5%a5%bd%ef%bc%8c%e4%b8%96%e7%95%8c%ef%bc%81 HTTP/1.1\r\n" // "txt1=hello&txt2=world&a=你好，世界！"
        "Content-Type:application/x-www-form-urlencoded\r\n"
        "Accept-Encoding: gzip, deflate\r\n"
        "Host: w.sohu.com\r\n"
        "Connection: Keep-Alive\r\n"
        "Cache-Control: no-cache\r\n"
        "\r\n";

    http_parser_.Execute(post_req, strlen(post_req));
}

void FormTest::Test003()
{
#define BOUNDARY "simple boundary"
#define PART0_BODY                                      \
    "This is implicitly typed plain ASCII text.\r\n"    \
    "It does NOT end with a linebreak."
#define PART1_BODY                                      \
    "This is explicitly typed plain ASCII text.\r\n"    \
    "It DOES end with a linebreak.\r\n"
#define BODY                                            \
    "--" BOUNDARY "\r\n"                                \
    "\r\n"                                              \
    PART0_BODY                                          \
    "\r\n--" BOUNDARY "\r\n"                            \
    "Content-type: text/plain; charset=us-ascii\r\n"    \
    "\r\n"                                              \
    PART1_BODY                                          \
    "\r\n--" BOUNDARY "--\r\n"                          \

    const char post_req[] =
        "POST / HTTP/1.1\r\n"
        "Content-Length: 60408\r\n"
        "Content-Type:multipart/form-data; boundary=" BOUNDARY "\r\n"
        "Host: www.qq.com\r\n"
        "\r\n"
        BODY;

    http_parser_.Execute(post_req, strlen(post_req));

#undef BOUNDARY
#undef PART0_BODY
#undef PART1_BODY
#undef BODY
}

void FormTest::Test004()
{
#define BOUNDARY "simple boundary"
#define PART0_BODY                                      \
    "This is implicitly typed plain ASCII text.\r\n"    \
    "It does NOT end with a linebreak."
#define PART1_BODY                                      \
    "This is explicitly typed plain ASCII text.\r\n"    \
    "It DOES end with a linebreak.\r\n"
#define BODY                                            \
    "This is the preamble.  It is to be ignored, though it\r\n" \
    "is a handy place for mail composers to include an\r\n"     \
    "explanatory note to non-MIME compliant readers.\r\n"       \
    "--" BOUNDARY "\r\n"                                \
    "\r\n"                                              \
    PART0_BODY                                          \
    "\r\n--" BOUNDARY "\r\n"                            \
    "Content-type: text/plain; charset=us-ascii\r\n"    \
    "\r\n"                                              \
    PART1_BODY                                          \
    "\r\n--" BOUNDARY "--\r\n"                          \

    const char post_req[] =
        "POST / HTTP/1.1\r\n"
        "Content-Length: 60408\r\n"
        "Content-Type:multipart/form-data; boundary=" BOUNDARY "\r\n"
        "Host: www.qq.com\r\n"
        "\r\n"
        BODY;

    http_parser_.Execute(post_req, strlen(post_req));

#undef BOUNDARY
#undef PART0_BODY
#undef PART1_BODY
#undef BODY
}

void FormTest::Test005()
{
#define BOUNDARY "simple boundary"
#define PART0_BODY                                      \
    "This is implicitly typed plain ASCII text.\r\n"    \
    "It does NOT end with a linebreak."
#define PART1_BODY                                      \
    "This is explicitly typed plain ASCII text.\r\n"    \
    "It DOES end with a linebreak.\r\n"
#define BODY                                            \
    "--" BOUNDARY "\r\n"                                \
    "\r\n"                                              \
    PART0_BODY                                          \
    "\r\n--" BOUNDARY "\r\n"                            \
    "Content-type: text/plain; charset=us-ascii\r\n"    \
    "\r\n"                                              \
    PART1_BODY                                          \
    "\r\n--" BOUNDARY "--\r\n"                          \
    "This is the epilogue.  It is to be ignored.\r\n"           \

    const char post_req[] =
        "POST / HTTP/1.1\r\n"
        "Content-Length: 60408\r\n"
        "Content-Type:multipart/form-data; boundary=" BOUNDARY "\r\n"
        "Host: www.qq.com\r\n"
        "\r\n"
        BODY;

    http_parser_.Execute(post_req, strlen(post_req));

#undef BOUNDARY
#undef PART0_BODY
#undef PART1_BODY
#undef BODY
}

void FormTest::Test006()
{
#define BOUNDARY "simple boundary"
#define PART0_BODY                                      \
    "Content-Disposition: form-data; name=\"foo\"; filename=\"bar\"\r\n"    \
    "Content-Type: application/octet-stream\r\n"                            \
    "\r\n"                                                                  \
    "That's the file content!\r\n"
#define PART1_BODY                                      \
    "Content-Disposition: form-data; name=\"foo\"\r\n"  \
    "\r\n"                                              \
    "bar"
#define BODY                                            \
    "--" BOUNDARY "\r\n"                                \
    "\r\n"                                              \
    PART0_BODY                                          \
    "\r\n--" BOUNDARY "\r\n"                            \
    "Content-type: text/plain; charset=us-ascii\r\n"    \
    "\r\n"                                              \
    PART1_BODY                                          \
    "\r\n--" BOUNDARY "--\r\n"                          \

    const char post_req[] =
        "POST / HTTP/1.1\r\n"
        "Content-Length: 60408\r\n"
        "Content-Type:multipart/form-data; boundary=" BOUNDARY "\r\n"
        "Host: www.qq.com\r\n"
        "\r\n"
        BODY;

    http_parser_.Execute(post_req, strlen(post_req));

#undef BOUNDARY
#undef PART0_BODY
#undef PART1_BODY
#undef BODY
}

void FormTest::Test007()
{
#define BOUNDARY "simple boundary"
#define PART0_BODY ""
#define PART1_BODY                                      \
    "Content-Disposition: form-data; name=\"foo\"\r\n"  \
    "\r\n"                                              \
    "bar"
#define BODY                                            \
    "--" BOUNDARY "\r\n"                                \
    "\r\n"                                              \
    PART0_BODY                                          \
    "\r\n--" BOUNDARY "\r\n"                            \
    "Content-type: text/plain; charset=us-ascii\r\n"    \
    "\r\n"                                              \
    PART1_BODY                                          \
    "\r\n--" BOUNDARY "--\r\n"                          \

    const char post_req[] =
        "POST / HTTP/1.1\r\n"
        "Content-Length: 60408\r\n"
        "Content-Type:multipart/form-data; boundary=" BOUNDARY "\r\n"
        "Host: www.qq.com\r\n"
        "\r\n"
        BODY;

    http_parser_.Execute(post_req, strlen(post_req));

#undef BOUNDARY
#undef PART0_BODY
#undef PART1_BODY
#undef BODY
}

void FormTest::Test008()
{
#define BOUNDARY "simple boundary"
#define PART0_BODY "Content-Di"
#define PART1_BODY                                      \
    "sposition: form-data; name=\"foo\"\r\n"            \
    "\r\n"                                              \
    "bar"
#define BODY                                            \
    "--" BOUNDARY "\r\n"                                \
    "\r\n"                                              \
    PART0_BODY                                          \
    "\r\n--" BOUNDARY "\r\n"                            \
    "Content-type: text/plain; charset=us-ascii\r\n"    \
    "\r\n"                                              \
    PART1_BODY                                          \
    "\r\n--" BOUNDARY "--\r\n"                          \

    const char post_req[] =
        "POST / HTTP/1.1\r\n"
        "Content-Length: 60408\r\n"
        "Content-Type:multipart/form-data; boundary=" BOUNDARY "\r\n"
        "Host: www.qq.com\r\n"
        "\r\n"
        BODY;

    http_parser_.Execute(post_req, strlen(post_req));

#undef BOUNDARY
#undef PART0_BODY
#undef PART1_BODY
#undef BODY
}

void FormTest::Test009()
{
#define BOUNDARY "simple boundary"
#define PART0_BODY "Content-Disposition"
#define PART1_BODY                                      \
    ": form-data; name=\"foo\"\r\n"                     \
    "\r\n"                                              \
    "bar"
#define BODY                                            \
    "--" BOUNDARY "\r\n"                                \
    "\r\n"                                              \
    PART0_BODY                                          \
    "\r\n--" BOUNDARY "\r\n"                            \
    "Content-type: text/plain; charset=us-ascii\r\n"    \
    "\r\n"                                              \
    PART1_BODY                                          \
    "\r\n--" BOUNDARY "--\r\n"                          \

    const char post_req[] =
        "POST / HTTP/1.1\r\n"
        "Content-Length: 60408\r\n"
        "Content-Type:multipart/form-data; boundary=" BOUNDARY "\r\n"
        "Host: www.qq.com\r\n"
        "\r\n"
        BODY;

    http_parser_.Execute(post_req, strlen(post_req));

#undef BOUNDARY
#undef PART0_BODY
#undef PART1_BODY
#undef BODY
}

void FormTest::Test0010()
{
#define BOUNDARY "simple boundary"
#define PART0_BODY "Content-Disposition: "
#define PART1_BODY                                      \
    "form-data; name=\"foo\"\r\n"                       \
    "\r\n"                                              \
    "bar"
#define BODY                                            \
    "--" BOUNDARY "\r\n"                                \
    "\r\n"                                              \
    PART0_BODY                                          \
    "\r\n--" BOUNDARY "\r\n"                            \
    "Content-type: text/plain; charset=us-ascii\r\n"    \
    "\r\n"                                              \
    PART1_BODY                                          \
    "\r\n--" BOUNDARY "--\r\n"                          \

    const char post_req[] =
        "POST / HTTP/1.1\r\n"
        "Content-Length: 60408\r\n"
        "Content-Type:multipart/form-data; boundary=" BOUNDARY "\r\n"
        "Host: www.qq.com\r\n"
        "\r\n"
        BODY;

    http_parser_.Execute(post_req, strlen(post_req));

#undef BOUNDARY
#undef PART0_BODY
#undef PART1_BODY
#undef BODY
}

void FormTest::Test0011()
{
#define BOUNDARY "simple boundary"
#define PART0_BODY "Content-Disposition: form-data"
#define PART1_BODY                                      \
    "; name=\"foo\"\r\n"                                \
    "\r\n"                                              \
    "bar"
#define BODY                                            \
    "--" BOUNDARY "\r\n"                                \
    "\r\n"                                              \
    PART0_BODY                                          \
    "\r\n--" BOUNDARY "\r\n"                            \
    "Content-type: text/plain; charset=us-ascii\r\n"    \
    "\r\n"                                              \
    PART1_BODY                                          \
    "\r\n--" BOUNDARY "--\r\n"                          \

    const char post_req[] =
        "POST / HTTP/1.1\r\n"
        "Content-Length: 60408\r\n"
        "Content-Type:multipart/form-data; boundary=" BOUNDARY "\r\n"
        "Host: www.qq.com\r\n"
        "\r\n"
        BODY;

    http_parser_.Execute(post_req, strlen(post_req));

#undef BOUNDARY
#undef PART0_BODY
#undef PART1_BODY
#undef BODY
}

ADD_TEST_F(FormTest, Test001);
ADD_TEST_F(FormTest, Test002);
ADD_TEST_F(FormTest, Test003);
ADD_TEST_F(FormTest, Test004);
ADD_TEST_F(FormTest, Test005);
ADD_TEST_F(FormTest, Test006);
ADD_TEST_F(FormTest, Test007);
ADD_TEST_F(FormTest, Test008);
ADD_TEST_F(FormTest, Test009);
ADD_TEST_F(FormTest, Test0010);
ADD_TEST_F(FormTest, Test0011);
}
}
