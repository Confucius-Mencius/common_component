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

void Test002()
{
    printf("%s| %s| %s| %s| %d| %s\n", __func__, __FUNCTION__, __PRETTY_FUNCTION__, __FILE__, __LINE__, basename(__FILE__));
}

ADD_TEST(SimpleLogTest, Test001);
ADD_TEST(SimpleLogTest, Test002);
}
