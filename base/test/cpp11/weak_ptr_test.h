#ifndef BASE_TEST_SMART_PTR_WEAK_PTR_TEST_H_
#define BASE_TEST_SMART_PTR_WEAK_PTR_TEST_H_

#include "test_util.h"

class WeakPtrTest : public GTest
{
public:
    WeakPtrTest();
    virtual ~WeakPtrTest();

    void Test001();
    void Test002();
    void Test003();

private:

};

#endif // BASE_TEST_SMART_PTR_WEAK_PTR_TEST_H_
