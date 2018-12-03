#ifndef EVENT_CENTER_TEST_MOCK_ACTION_EVENT_SINK_H_
#define EVENT_CENTER_TEST_MOCK_ACTION_EVENT_SINK_H_

#include <gmock/gmock.h>
#include "event_center_interface.h"

using ::testing::_;
using ::testing::Invoke;

// 什么都不做，用于测试调用行为
class MockActionEventSink1 : public ActionEventSinkInterface
{
public:
    MOCK_METHOD5(OnAction,
                 void(int event_category, int event_id, void* event_provider, void* event_ctx, int times));
};

// 在回调的时候退订自己
class FakeActionEventSink2 : public ActionEventSinkInterface
{
public:
    void SetCtx(EventCenterInterface* event_center, void* event_provider, ActionEventSinkInterface* event_sink)
    {
        event_center_ = event_center;
        event_provider_ = event_provider;
        event_sink_ = event_sink;
    }

    virtual void OnAction(int event_category, int event_id, void* event_provider, void* event_ctx, int times)
    {
        if (NULL == event_center_)
        {
            return;
        }

        event_center_->RemoveActionListener(event_category, event_id, event_provider_, event_sink_);
    }

private:
    EventCenterInterface* event_center_;
    void* event_provider_;
    ActionEventSinkInterface* event_sink_;
};

class MockActionEventSink2 : public ActionEventSinkInterface
{
public:
    MOCK_METHOD5(OnAction,
                 void(int event_category, int event_id, void* event_provider, void* event_ctx, int times));

    void Delegate(EventCenterInterface* event_center, void* event_provider)
    {
        fake_action_event_sink_2_.SetCtx(event_center, event_provider, this);
        ON_CALL(*this, OnAction(_, _, _, _, _)).WillByDefault(Invoke(&fake_action_event_sink_2_, &FakeActionEventSink2::OnAction));
    }

private:
    FakeActionEventSink2 fake_action_event_sink_2_;
};

// 在回调的时候先退订自己，然后再订阅自己，反复进行
class FakeActionEventSink3 : public ActionEventSinkInterface
{
public:
    void SetCtx(EventCenterInterface* event_center, void* event_provider, ActionEventSinkInterface* event_sink)
    {
        event_center_ = event_center;
        event_provider_ = event_provider;
        event_sink_ = event_sink;
    }

    virtual void OnAction(int event_category, int event_id, void* event_provider, void* event_ctx, int times)
    {
        if (NULL == event_center_)
        {
            return;
        }

        event_center_->RemoveActionListener(event_category, event_id, event_provider_, event_sink_);

        if (event_center_->AddActionListener(event_category, event_id, event_provider_, event_sink_, -1) != 0)
        {
            assert(0);
        }
    }

private:
    EventCenterInterface* event_center_;
    void* event_provider_;
    ActionEventSinkInterface* event_sink_;
};

class MockActionEventSink3 : public ActionEventSinkInterface
{
public:
    MOCK_METHOD5(OnAction,
                 void(int event_category, int event_id, void* event_provider, void* event_ctx, int times));

    void Delegate(EventCenterInterface* event_center, void* event_provider)
    {
        fake_action_event_sink_3_.SetCtx(event_center, event_provider, this);
        ON_CALL(*this, OnAction(_, _, _, _, _)).WillByDefault(Invoke(&fake_action_event_sink_3_, &FakeActionEventSink3::OnAction));
    }

private:
    FakeActionEventSink3 fake_action_event_sink_3_;
};

// 在回调的时候退订其它的监听者
class FakeActionEventSink4 : public ActionEventSinkInterface
{
public:
    void SetCtx(EventCenterInterface* event_center, void* other_provider, ActionEventSinkInterface* other_sink)
    {
        event_center_ = event_center;
        other_provider_ = other_provider;
        other_sink_ = other_sink;
    }

    virtual void OnAction(int event_category, int event_id, void* event_provider, void* event_ctx, int times)
    {
        if (NULL == event_center_)
        {
            return;
        }

        event_center_->RemoveActionListener(event_category, event_id, other_provider_, other_sink_);
    }

private:
    EventCenterInterface* event_center_;
    void* other_provider_;
    ActionEventSinkInterface* other_sink_;
};

class MockActionEventSink4 : public ActionEventSinkInterface
{
public:
    MOCK_METHOD5(OnAction,
                 void(int event_category, int event_id, void* event_provider, void* event_ctx, int times));

    void Delegate(EventCenterInterface* event_center, void* other_provider, ActionEventSinkInterface* other_sink)
    {
        fake_action_event_sink_4_.SetCtx(event_center, other_provider, other_sink);
        ON_CALL(*this, OnAction(_, _, _, _, _)).WillByDefault(Invoke(&fake_action_event_sink_4_, &FakeActionEventSink4::OnAction));
    }

private:
    FakeActionEventSink4 fake_action_event_sink_4_;
};

// 在回调的时候订阅其他的监听者
class FakeActionEventSink5 : public ActionEventSinkInterface
{
public:
    void SetCtx(EventCenterInterface* event_center, void* other_provider, ActionEventSinkInterface* other_sink)
    {
        event_center_ = event_center;
        other_provider_ = other_provider;
        other_sink_ = other_sink;
    }

    virtual void OnAction(int event_category, int event_id, void* event_provider, void* event_ctx, int times)
    {
        if (NULL == event_center_)
        {
            return;
        }

        if (event_center_->ActionListenerExist(event_category, event_id, other_provider_, other_sink_))
        {
            return;
        }

        if (event_center_->AddActionListener(event_category, event_id, other_provider_, other_sink_, -1) != 0)
        {
            assert(0);
        }
    }

private:
    EventCenterInterface* event_center_;
    void* other_provider_;
    ActionEventSinkInterface* other_sink_;
};

class MockActionEventSink5 : public ActionEventSinkInterface
{
public:
    MOCK_METHOD5(OnAction,
                 void(int event_category, int event_id, void* event_provider, void* event_ctx, int times));

    void Delegate(EventCenterInterface* event_center, void* other_provider, ActionEventSinkInterface* other_sink)
    {
        fake_action_event_sink_5_.SetCtx(event_center, other_provider, other_sink);
        ON_CALL(*this, OnAction(_, _, _, _, _)).WillByDefault(Invoke(&fake_action_event_sink_5_, &FakeActionEventSink5::OnAction));
    }

private:
    FakeActionEventSink5 fake_action_event_sink_5_;
};

#endif // EVENT_CENTER_TEST_MOCK_ACTION_EVENT_SINK_H_
