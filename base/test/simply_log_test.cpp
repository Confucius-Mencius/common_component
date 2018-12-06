#include "test_util.h"
#include "simple_log.h"

namespace simple_log_test
{
void Test001()
{
    LOG_C("%d-%s", 1, "hgc");
    LOG_CE("%d-%s", 1, "hgc");

    LOG_C("hello, world");
    LOG_CE("hello, world");

    LOG_CPP(1 << "-hgc");
    LOG_CPPE(1 << "-hgc");
}

ADD_TEST(SimplyLogTest, Test001);
}
