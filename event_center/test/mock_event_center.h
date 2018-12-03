#ifndef EVENT_CENTER_TEST_MOCK_EVENT_CENTER_H_
#define EVENT_CENTER_TEST_MOCK_EVENT_CENTER_H_

#include <gmock/gmock.h>
#include "event_center_interface.h"

// generate by c_third_party_lib/tools/gmock_generator/generate_mock.sh
class MockEventCenterInterface : public EventCenterInterface
{
public:
    MOCK_CONST_METHOD0(GetVersion,
                       const char*());
    MOCK_CONST_METHOD0(GetLastErrMsg,
                       const char*());
    MOCK_METHOD0(Release,
                 void());
    MOCK_METHOD1(Initialize,
                 int(const void* ctx));
    MOCK_METHOD0(Finalize,
                 void());
    MOCK_METHOD0(Activate,
                 int());
    MOCK_METHOD0(Freeze,
                 void());

public:
    MOCK_METHOD2(SetCategoryRelation,
                 int(int parent, int child));
    MOCK_CONST_METHOD4(VoteListenerExist,
                       bool(int event_category, int event_id, void* event_provider, VoteEventSinkInterface* event_sink));
    MOCK_METHOD5(AddVoteListener,
                 int(int event_category, int event_id, void* event_provider, VoteEventSinkInterface* event_sink, int total_times));
    MOCK_METHOD4(RemoveVoteListener,
                 void(int event_category, int event_id, void* event_provider, VoteEventSinkInterface* event_sink));
    MOCK_METHOD4(FireVoteEvent,
                 bool(int event_category, int event_id, void* event_provider, void* event_ctx));
    MOCK_CONST_METHOD4(ActionListenerExist,
                       bool(int event_category, int event_id, void* event_provider, ActionEventSinkInterface* event_sink));
    MOCK_METHOD5(AddActionListener,
                 int(int event_category, int event_id, void* event_provider, ActionEventSinkInterface* event_sink, int total_times));
    MOCK_METHOD4(RemoveActionListener,
                 void(int event_category, int event_id, void* event_provider, ActionEventSinkInterface* event_sink));
    MOCK_METHOD4(FireActionEvent,
                 void(int event_category, int event_id, void* event_provider, void* event_ctx));
};

class MockVoteEventSinkInterface : public VoteEventSinkInterface
{
public:
    MOCK_METHOD5(OnVote,
                 bool(int event_category, int event_id, void* event_provider, void* event_ctx, int times));
};

class MockActionEventSinkInterface : public ActionEventSinkInterface
{
public:
    MOCK_METHOD5(OnAction,
                 void(int event_category, int event_id, void* event_provider, void* event_ctx, int times));
};

class MockEventSinkInterface : public VoteEventSinkInterface, public ActionEventSinkInterface
{
public:
    MOCK_METHOD5(OnVote,
                 bool(int event_category, int event_id, void* event_provider, void* event_ctx, int times));

public:
    MOCK_METHOD5(OnAction,
                 void(int event_category, int event_id, void* event_provider, void* event_ctx, int times));
};

#endif // EVENT_CENTER_TEST_MOCK_EVENT_CENTER_H_
