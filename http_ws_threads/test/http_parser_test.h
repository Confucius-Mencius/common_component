#ifndef HTTP_WS_THREADS_TEST_HTTP_PARSER_TEST_H_
#define HTTP_WS_THREADS_TEST_HTTP_PARSER_TEST_H_

#include "the_http_parser.h"
#include "test_util.h"

namespace tcp
{
namespace http_ws
{
class HTTPParserTest : public GTest
{
public:
    HTTPParserTest();
    virtual ~HTTPParserTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
    void Test005();
    void Test006();

private:
    tcp::http_ws::http::Parser http_parser_;
};
}
}

#endif // HTTP_WS_THREADS_TEST_HTTP_PARSER_TEST_H_
