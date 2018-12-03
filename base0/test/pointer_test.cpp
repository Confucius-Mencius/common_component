#ifndef BASE_TEST_POINTER_TEST_H_
#define BASE_TEST_POINTER_TEST_H_

#include "test_util.h"

namespace pointer_test
{
struct Ctx
{
    int* p;

    Ctx()
    {
        p = NULL;
    }
};

class A
{
public:
    A() : ctx_()
    {
        p_ = NULL;
    }

    ~A()
    {
    }

    int Initialize()
    {
        ctx_.p = p_;
        std::cout << ctx_.p << std::endl;
        p_ = new int();
        std::cout << ctx_.p << std::endl;
        return 0;
    }

    void Finalize()
    {
        if (p_ != NULL)
        {
            delete p_;
            p_ = NULL;
        }
    }

private:
    int* p_;
    Ctx ctx_;
};

void Test001()
{
    A a;
    a.Initialize();
    a.Finalize();
}

ADD_TEST(PointerTest, Test001);
}

#endif // BASE_TEST_POINTER_TEST_H_
