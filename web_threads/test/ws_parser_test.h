#ifndef WEB_THREADS_TEST_WS_PARSER_TEST_H_
#define WEB_THREADS_TEST_WS_PARSER_TEST_H_

#include "the_ws_parser.h"
#include "test_util.h"

namespace tcp
{
namespace web
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
    tcp::web::ws::Parser ws_parser_;
};
}
}

#endif // WEB_THREADS_TEST_WS_PARSER_TEST_H_
