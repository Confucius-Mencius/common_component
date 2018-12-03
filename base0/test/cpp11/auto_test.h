#ifndef BASE_TEST_CPP11_AUTO_TEST_H_
#define BASE_TEST_CPP11_AUTO_TEST_H_

#include "test_util.h"

class AutoTest : public GTest
{
public:
    AutoTest();
    virtual ~AutoTest();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
    void Test005();

private:

};

#endif // BASE_TEST_CPP11_AUTO_TEST_H_
