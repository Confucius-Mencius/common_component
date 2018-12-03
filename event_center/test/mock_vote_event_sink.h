#ifndef EVENT_CENTER_TEST_MOCK_VOTE_EVENT_SINK_H_
#define EVENT_CENTER_TEST_MOCK_VOTE_EVENT_SINK_H_

#include <gmock/gmock.h>
#include "event_center_interface.h"

using ::testing::_;
using ::testing::Invoke;

// 什么都不做，用于测试调用行为
class MockVoteEventSink1 : public VoteEventSinkInterface
{
public:
    MOCK_METHOD5(OnVote,
                 bool(int event_category, int event_id, void* event_provider, void* event_ctx, int times));
};

// 在回调的时候退订自己
class FakeVoteEventSink2 : public VoteEventSinkInterface
{
public:
    void SetCtx(EventCenterInterface* event_center, void* event_provider, VoteEventSinkInterface* event_sink)
    {
        event_center_ = event_center;
        event_provider_ = event_provider;
        event_sink_ = event_sink;
    }

    virtual bool OnVote(int event_category, int event_id, void* event_provider, void* event_ctx, int times)
    {
        if (NULL == event_center_)
        {
            return false;
        }

        event_center_->RemoveVoteListener(event_category, event_id, event_provider_, event_sink_);
        return true;
    }

private:
    EventCenterInterface* event_center_;
    void* event_provider_;
    VoteEventSinkInterface* event_sink_;
};

class MockVoteEventSink2 : public VoteEventSinkInterface
{
public:
    MOCK_METHOD5(OnVote,
                 bool(int event_category, int event_id, void* event_provider, void* event_ctx, int times));

    void Delegate(EventCenterInterface* event_center, void* event_provider)
    {
        fake_vote_event_sink_2_.SetCtx(event_center, event_provider, this);
        ON_CALL(*this, OnVote(_, _, _, _, _)).WillByDefault(Invoke(&fake_vote_event_sink_2_, &FakeVoteEventSink2::OnVote));
    }

private:
    FakeVoteEventSink2 fake_vote_event_sink_2_;
};

// 在回调的时候先退订自己，然后再订阅自己，反复进行
class FakeVoteEventSink3 : public VoteEventSinkInterface
{
public:
    void SetCtx(EventCenterInterface* event_center, void* event_provider, VoteEventSinkInterface* event_sink)
    {
        event_center_ = event_center;
        event_provider_ = event_provider;
        event_sink_ = event_sink;
    }

    virtual bool OnVote(int event_category, int event_id, void* event_provider, void* event_ctx, int times)
    {
        if (NULL == event_center_)
        {
            return false;
        }

        event_center_->RemoveVoteListener(event_category, event_id, event_provider_, event_sink_);

        if (event_center_->AddVoteListener(event_category, event_id, event_provider_, event_sink_, -1) != 0)
        {
            return false;
        }

        return true;
    }

private:
    EventCenterInterface* event_center_;
    void* event_provider_;
    VoteEventSinkInterface* event_sink_;
};

class MockVoteEventSink3 : public VoteEventSinkInterface
{
public:
    MOCK_METHOD5(OnVote,
                 bool(int event_category, int event_id, void* event_provider, void* event_ctx, int times));

    void Delegate(EventCenterInterface* event_center, void* event_provider)
    {
        fake_vote_event_sink_3_.SetCtx(event_center, event_provider, this);
        ON_CALL(*this, OnVote(_, _, _, _, _)).WillByDefault(Invoke(&fake_vote_event_sink_3_, &FakeVoteEventSink3::OnVote));
    }

private:
    FakeVoteEventSink3 fake_vote_event_sink_3_;
};

// 在回调的时候退订其它的监听者
class FakeVoteEventSink4 : public VoteEventSinkInterface
{
public:
    void SetCtx(EventCenterInterface* event_center, void* other_provider, VoteEventSinkInterface* other_sink)
    {
        event_center_ = event_center;
        other_provider_ = other_provider;
        other_sink_ = other_sink;
    }

    virtual bool OnVote(int event_category, int event_id, void* event_provider, void* event_ctx, int times)
    {
        if (NULL == event_center_)
        {
            return false;
        }

        event_center_->RemoveVoteListener(event_category, event_id, other_provider_, other_sink_);
        return true;
    }

private:
    EventCenterInterface* event_center_;
    void* other_provider_;
    VoteEventSinkInterface* other_sink_;
};

class MockVoteEventSink4 : public VoteEventSinkInterface
{
public:
    MOCK_METHOD5(OnVote,
                 bool(int event_category, int event_id, void* event_provider, void* event_ctx, int times));

    void Delegate(EventCenterInterface* event_center, void* other_provider, VoteEventSinkInterface* other_sink)
    {
        fake_vote_event_sink_4_.SetCtx(event_center, other_provider, other_sink);
        ON_CALL(*this, OnVote(_, _, _, _, _)).WillByDefault(Invoke(&fake_vote_event_sink_4_, &FakeVoteEventSink4::OnVote));
    }

private:
    FakeVoteEventSink4 fake_vote_event_sink_4_;
};

// 在回调的时候订阅其他的监听者
class FakeVoteEventSink5 : public VoteEventSinkInterface
{
public:
    void SetCtx(EventCenterInterface* event_center, void* other_provider, VoteEventSinkInterface* other_sink)
    {
        event_center_ = event_center;
        other_provider_ = other_provider;
        other_sink_ = other_sink;
    }

    virtual bool OnVote(int event_category, int event_id, void* event_provider, void* event_ctx, int times)
    {
        if (NULL == event_center_)
        {
            return false;
        }

        if (event_center_->VoteListenerExist(event_category, event_id, other_provider_, other_sink_))
        {
            return true;
        }

        if (event_center_->AddVoteListener(event_category, event_id, other_provider_, other_sink_, -1) != 0)
        {
            return false;
        }

        return true;
    }

private:
    EventCenterInterface* event_center_;
    void* other_provider_;
    VoteEventSinkInterface* other_sink_;
};

class MockVoteEventSink5 : public VoteEventSinkInterface
{
public:
    MOCK_METHOD5(OnVote,
                 bool(int event_category, int event_id, void* event_provider, void* event_ctx, int times));

    void Delegate(EventCenterInterface* event_center, void* other_provider, VoteEventSinkInterface* other_sink)
    {
        fake_vote_event_sink_5_.SetCtx(event_center, other_provider, other_sink);
        ON_CALL(*this, OnVote(_, _, _, _, _)).WillByDefault(Invoke(&fake_vote_event_sink_5_, &FakeVoteEventSink5::OnVote));
    }

private:
    FakeVoteEventSink5 fake_vote_event_sink_5_;
};

#endif // EVENT_CENTER_TEST_MOCK_VOTE_EVENT_SINK_H_
