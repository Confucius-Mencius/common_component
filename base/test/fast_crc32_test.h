#ifndef BASE_TEST_FAST_CRC32_TEST_H_
#define BASE_TEST_FAST_CRC32_TEST_H_

#include "test_util.h"

namespace fast_crc32_test
{
class FastCRC32Test : public GTest
{
public:
    FastCRC32Test();
    virtual ~FastCRC32Test();

    void Test001();
    void Test002();
};
}

#endif // BASE_TEST_FAST_CRC32_TEST_H_
