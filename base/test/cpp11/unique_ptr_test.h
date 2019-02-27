#ifndef BASE_TEST_SMART_PTR_UNIQUE_PTR_TEST_H_
#define BASE_TEST_SMART_PTR_UNIQUE_PTR_TEST_H_

#include "test_util.h"

class UniquePtrTest : public GTest
{
public:
    UniquePtrTest();
    virtual ~UniquePtrTest();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
    void TestAutoDestroy();
    void TestOwner();
    void TestArrayOwner();

private:

};

#endif // BASE_TEST_SMART_PTR_UNIQUE_PTR_TEST_H_
