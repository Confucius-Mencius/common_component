#ifndef THREAD_CENTER_TEST_THREAD_CENTER_TEST_H_
#define THREAD_CENTER_TEST_THREAD_CENTER_TEST_H_

#include "test_util.h"
#include "module_loader.h"
#include "thread_center_interface.h"
#include "thread_sink.h"

namespace thread_center_test
{
class ThreadCenterTest : public GTest
{
public:
    ThreadCenterTest();
    virtual ~ThreadCenterTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
    void Test003();
    void Test004();

private:
    ModuleLoader thread_center_loader_;
    ThreadCenterInterface* thread_center_;
};
}

#endif // THREAD_CENTER_TEST_THREAD_CENTER_TEST_H_
