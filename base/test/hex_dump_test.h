#ifndef BASE_TEST_HEX_DUMP_TEST_H_
#define BASE_TEST_HEX_DUMP_TEST_H_

#include "test_util.h"

namespace hex_dump_test
{
class HexDumpTest : public GTest
{
public:
    HexDumpTest();
    virtual ~HexDumpTest();

    void Test001();
    void Test002();
};
}

#endif // BASE_TEST_HEX_DUMP_TEST_H_
