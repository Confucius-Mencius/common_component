#ifndef BASE_TEST_MISC_TEST_H_
#define BASE_TEST_MISC_TEST_H_

#include "test_util.h"

namespace misc_test
{
class MiscTest : public GTest
{
public:
    MiscTest();
    virtual ~MiscTest();

    void LittleEndianedTest();
    void GccVersionTest();
};
}

#endif // BASE_TEST_MISC_TEST_H_
