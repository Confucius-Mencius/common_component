#ifndef EVENT_CENTER_TEST_EVENT_CENTER_TEST_H_
#define EVENT_CENTER_TEST_EVENT_CENTER_TEST_H_

#include "event_center_interface.h"
#include "module_loader.h"
#include "test_util.h"

class EventCenterTest : public GTest
{
public:
    EventCenterTest();
    virtual ~EventCenterTest();

    virtual void SetUp();
    virtual void TearDown();

    void VoteEventTest001();
    void VoteEventTest002();
    void VoteEventTest003();
    void VoteEventTest004();
    void VoteEventTest005();
    void VoteEventTest006();
    void VoteEventTest007();
    void VoteEventTest008();
    void VoteEventTest009();
    void VoteEventTest010();
    void ActionEventTest001();
    void ActionEventTest002();
    void ActionEventTest003();
    void ActionEventTest004();
    void ActionEventTest005();
    void ActionEventTest006();
    void ActionEventTest007();
    void ActionEventTest008();
    void ActionEventTest009();
    void Test001();
    void Test002();
    void RemoveTest001(); // 调用过程中删除测试
    void RemoveTest002(); // 调用过程中删除测试

    void LoadSoNumTest();

private:
    ModuleLoader loader_;
    EventCenterInterface* event_center_;
};

#endif // EVENT_CENTER_TEST_EVENT_CENTER_TEST_H_
