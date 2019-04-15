#ifndef HTTP_WS_THREADS_TEST_WS_PARSER_TEST_H_
#define HTTP_WS_THREADS_TEST_WS_PARSER_TEST_H_

#include "the_ws_parser.h"
#include "test_util.h"

namespace tcp
{
namespace http_ws
{
class WSParserTest : public GTest
{
public:
    WSParserTest();
    virtual ~WSParserTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
    void Test003();
    void Test004();

private:
    tcp::http_ws::ws::Parser ws_parser_;
};
}
}

#endif // HTTP_WS_THREADS_TEST_WS_PARSER_TEST_H_
