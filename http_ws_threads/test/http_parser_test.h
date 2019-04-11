#ifndef CONF_CENTER_TEST_CONF_CENTER_TEST_H_
#define CONF_CENTER_TEST_CONF_CENTER_TEST_H_

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

private:
    tcp::http_ws::http::Parser http_parser_;
};
}
}

#endif // CONF_CENTER_TEST_CONF_CENTER_TEST_H_
