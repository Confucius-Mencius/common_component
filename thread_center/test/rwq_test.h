#ifndef THREAD_CENTER_TEST_RWQ_TEST_H
#define THREAD_CENTER_TEST_RWQ_TEST_H

#include "test_util.h"

namespace thread_center_test
{
class RWQTest : public GTest
{
public:
    RWQTest();
    virtual ~RWQTest();

    void Test001();
    void Test002();
};
}

#endif // THREAD_CENTER_TEST_RWQ_TEST_H
