#ifndef BASE_TEST_MISC_TEST_H_
#define BASE_TEST_MISC_TEST_H_

#include "test_util.h"

class MiscTest : public GTest
{
public:
    MiscTest();
    virtual ~MiscTest();

    void LittleEndianTest();
    void atoi_Test001();

    void ExecShellCmdTest();

private:

};

#endif // BASE_TEST_MISC_TEST_H_
