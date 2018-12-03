#ifndef BASE_TEST_CPP11_SHARED_PTR_TEST_H_
#define BASE_TEST_CPP11_SHARED_PTR_TEST_H_

#include "test_util.h"

class SharedPtrTest : public GTest
{
public:
    SharedPtrTest();
    virtual ~SharedPtrTest();

    void Test001();
    void TestLoopReference();

private:

};

#endif // BASE_TEST_CPP11_SHARED_PTR_TEST_H_
