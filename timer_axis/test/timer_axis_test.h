#ifndef TIMER_AXIS_TEST_TIMER_AXIS_TEST_H_
#define TIMER_AXIS_TEST_TIMER_AXIS_TEST_H_

#include <event2/event_struct.h>
#include "module_loader.h"
#include "test_util.h"
#include "timer_axis_interface.h"

class TimerAxisTest : public GTest
{
public:
    TimerAxisTest();
    virtual ~TimerAxisTest();

    virtual void SetUp();
    virtual void TearDown();

    void Test001();
    void Test002();
    void Test003();
    void Test004();
    void Test005();
    void Test006();
    void Test007();

private:
    struct event_base* event_base_;
    ModuleLoader loader_; // 这个对象里面要保存模块的信息，必须在进程的生存期都有效，不能使用局部变量
    TimerAxisInterface* timer_axis_;
};

#endif // TIMER_AXIS_TEST_TIMER_AXIS_TEST_H_
