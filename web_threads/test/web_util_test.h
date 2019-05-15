#ifndef WEB_THREADS_TEST_WEB_UTIL_TEST_H_
#define WEB_THREADS_TEST_WEB_UTIL_TEST_H_

#include "web_util.h"
#include "test_util.h"

namespace tcp
{
namespace web
{
class WebUtilTest : public GTest
{
public:
    WebUtilTest();
    virtual ~WebUtilTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
    void Test003();
};
}
}

#endif // WEB_THREADS_TEST_WEB_UTIL_TEST_H_
