#ifndef BASE_TEST_BOOST_TEST_H_
#define BASE_TEST_BOOST_TEST_H_

#include "test_util.h"

class BoostTest : public GTest
{
public:
    BoostTest();
    virtual ~BoostTest();

    void Test_Bind();
    void Test_Ptr();
    void Test_Locale(); // 编码转换
    void Test_CircularBuffer_int();
    void Test_CircularBuffer_char();
    void Test_CircularBuffer_ReloadSize();
    void Test_Split();
    void Test_Path1();
    void Test_Path2();

private:

};

#endif // BASE_TEST_BOOST_TEST_H_
