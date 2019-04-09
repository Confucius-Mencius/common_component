#ifndef CONF_CENTER_TEST_CONF_CENTER_TEST_H_
#define CONF_CENTER_TEST_CONF_CENTER_TEST_H_

#include "http.h"
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

private:
    tcp::http::HTTPParser http_parser_;
};
}
}

#endif // CONF_CENTER_TEST_CONF_CENTER_TEST_H_
