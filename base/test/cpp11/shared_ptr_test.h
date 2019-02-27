#ifndef BASE_TEST_SMART_PTR_SHARED_PTR_TEST_H_
#define BASE_TEST_SMART_PTR_SHARED_PTR_TEST_H_

#include "test_util.h"

class SharedPtrTest : public GTest
{
public:
    SharedPtrTest();
    virtual ~SharedPtrTest();

    void Test001();
    void TestLoopReference();
    void TestShareThisPointer();

private:

};

#endif // BASE_TEST_SMART_PTR_SHARED_PTR_TEST_H_
