#include "test_util.h"

static void Test001()
{
    printf("%d\n", __GNUC__);
    printf("%d\n", __GNUC_MINOR__);
    printf("%d\n", __GNUC_PATCHLEVEL__);
}

ADD_TEST(GCCVersionTest, Test001);
