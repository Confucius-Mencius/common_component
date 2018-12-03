#ifndef BASE_TEST_ZEROMQ_TEST_H_
#define BASE_TEST_ZEROMQ_TEST_H_

#include "test_util.h"

class ZeroMQTest : public GTest
{
public:
    ZeroMQTest();
    virtual ~ZeroMQTest();

    void Test001();

private:

};

#endif // BASE_TEST_ZEROMQ_TEST_H_
