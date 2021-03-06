#ifndef THREAD_CENTER_TEST_THREAD_CENTER_TEST_EX_H_
#define THREAD_CENTER_TEST_THREAD_CENTER_TEST_EX_H_

#include "module_loader.h"
#include "test_util.h"
#include "thread_sink_ex.h"

namespace thread_center_test
{
class ThreadCenterTestEx : public GTest
{
public:
    ThreadCenterTestEx();
    virtual ~ThreadCenterTestEx();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();

private:
    ModuleLoader loader_;
    ThreadCenterInterface* thread_center_;
};
}

#endif // THREAD_CENTER_TEST_THREAD_CENTER_TEST_EX_H_
