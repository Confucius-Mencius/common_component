#include "misc_test.h"
#include "misc_util.h"

#define LOG_WITH_CONTEXT 0
#include "simple_log.h"

namespace misc_test
{
MiscTest::MiscTest()
{
}

MiscTest::~MiscTest()
{
}

void MiscTest::LittleEndianedTest()
{
    EXPECT_TRUE(LITTLE_ENDIANED);
}

void MiscTest::GccVersionTest()
{
    LOG_C("%d", __GNUC__);
    LOG_C("%d", __GNUC_MINOR__);
    LOG_C("%d", __GNUC_PATCHLEVEL__);
}

ADD_TEST_F(MiscTest, LittleEndianedTest);
ADD_TEST_F(MiscTest, GccVersionTest);
}
