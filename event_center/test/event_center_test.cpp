#include "event_center_test.h"
#include "event_center_util.h"
#include "fake_event_define.h"
#include "mem_util.h"
#include "mock_event_center.h"
#include "mock_action_event_sink.h"
#include "mock_vote_event_sink.h"
#include "module_loader.h"

using ::testing::Return;
using testing::NiceMock;

EventCenterTest::EventCenterTest() : loader_()
{
    event_center_ = NULL;
}

EventCenterTest::~EventCenterTest()
{
}

void EventCenterTest::SetUp()
{
    if (loader_.Load("../libevent_center.so") != 0)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    event_center_ = (EventCenterInterface*) loader_.GetModuleInterface();
    if (NULL == event_center_)
    {
        FAIL() << loader_.GetLastErrMsg();
    }

    EventCenterCtx event_center_ctx;
    event_center_ctx.ncategories = fake_event::EVENT_CATEGORY_MAX;

    ASSERT_EQ(0, event_center_->Initialize(&event_center_ctx));
    ASSERT_EQ(0, event_center_->Activate());
}

void EventCenterTest::TearDown()
{
    SAFE_DESTROY_MODULE(event_center_, loader_);
}

/**
 * @brief Vote事件相关接口基本测试
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 *  - Expect:
 1，vote事件的检查、订阅、退订正常
 2，监听sink能收到发布的vote事件
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::VoteEventTest001()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    // 事件现场
    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockVoteEventSink1 sink1;

    EXPECT_CALL(sink1, OnVote(event_category, event_id, actor, &event_ctx_abc, 1)).Times(1);
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));

    // 订阅
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));

    // 退订
    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink1);
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));

    // 订阅
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));

    // 发布
    // mock布尔返回值的接口，默认返回false
    EXPECT_FALSE(event_center_->FireVoteEvent(event_category, event_id, actor, &event_ctx_abc));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));

    // 退订
    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink1);
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));

    delete actor;
}

/**
 * @brief Vote事件在回调的时候退订自身
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 *  - Expect:
 1，退订之后就收不到后续发布的事件了
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::VoteEventTest002()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockVoteEventSink1 sink1;
    MockVoteEventSink2 sink2;

    sink2.Delegate(event_center_, actor);

    const int loop = 100;

    // 默认返回false，这里做修改使其返回true
    EXPECT_CALL(sink1, OnVote(event_category, event_id, actor, &event_ctx_abc, _))
    .Times(loop).WillRepeatedly(Return(true));
    EXPECT_CALL(sink2, OnVote(event_category, event_id, actor, &event_ctx_abc, 1)).Times(1);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink2));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink2, -1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink2));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireVoteEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink2));

    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink1);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));

    delete actor;
}

/**
 * @brief Vote事件在回调的时候先退订自身，然后再订阅自身
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 *  - Expect:
 1，后续发布事件时仍能收到，且次数正确
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::VoteEventTest003()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockVoteEventSink1 sink1;
    MockVoteEventSink3 sink3;

    sink3.Delegate(event_center_, actor);

    const int loop = 100;

    // 默认返回false，这里做修改使其返回true
    EXPECT_CALL(sink1, OnVote(event_category, event_id, actor, &event_ctx_abc, _))
    .Times(loop).WillRepeatedly(Return(true));
    EXPECT_CALL(sink3, OnVote(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink3));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink3, -1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink3));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireVoteEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink3));

    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink3);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink3));

    delete actor;
}

/**
 * @brief Vote事件在回调的时候退订其他的某个事件（回调对象不同）
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 *  - Expect:
 1，后续发布事件时仍能收到，且次数正确
 2，被退订的回调对象收不到后续发布的事件了
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::VoteEventTest004()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockVoteEventSink1 sink1;
    MockVoteEventSink4 sink4;

    sink4.Delegate(event_center_, actor, &sink1);

    const int loop = 100;

    // 默认返回false，这里做修改使其返回true
    EXPECT_CALL(sink1, OnVote(event_category, event_id, actor, &event_ctx_abc, 1)).Times(1).WillOnce(Return(true));
    EXPECT_CALL(sink4, OnVote(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink4));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink4, -1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink4));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireVoteEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink4));

    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink4);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink4));

    delete actor;
}

/**
 * @brief Vote事件在回调的时候订阅其他的某个事件（回调对象不同）
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 *  - Expect:
 1，后续发布事件时仍能收到，且次数正确
 2，新增加的回调对象可以收到后续发布的事件，且次数正确
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::VoteEventTest005()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockVoteEventSink1 sink1;
    MockVoteEventSink5 sink5;

    sink5.Delegate(event_center_, actor, &sink1);

    const int loop = 100;

    // 默认返回false，这里做修改使其返回true
    EXPECT_CALL(sink1, OnVote(event_category, event_id, actor, &event_ctx_abc, _))
    .Times(loop - 1).WillRepeatedly(Return(true));
    EXPECT_CALL(sink5, OnVote(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink5));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink5, -1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink5));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireVoteEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink5));

    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink5);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink5));

    delete actor;
}

/**
 * @brief 事件发出者不为NULL时，订阅了事件发出者为NULL的监听者也能收到发布的该ID的Vote事件
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 2，监听者1订阅事件时指定了事件发出者
 3，监听者2订阅相同的事件时未指定事件发出者（为NULL）
 4，发布事件时，有具体的发出者（发出者不为NULL）
 *  - Expect:
 1，监听者1和监听者2都会收到该事件
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::VoteEventTest006()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockVoteEventSink1 sink1;
    MockVoteEventSink1 sink6;

    const int loop = 100;

    // 默认返回false，这里做修改使其返回true
    EXPECT_CALL(sink1, OnVote(event_category, event_id, actor, &event_ctx_abc, _))
    .Times(loop).WillRepeatedly(Return(true));
    EXPECT_CALL(sink6, OnVote(event_category, event_id, actor, &event_ctx_abc, _))
    .Times(loop).WillRepeatedly(Return(true));

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, NULL, &sink6));

    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, NULL, &sink6, -1));

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, NULL, &sink6));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireVoteEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, NULL, &sink6));

    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveVoteListener(event_category, event_id, NULL, &sink6);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, NULL, &sink6));

    delete actor;
}

/**
 * @brief 事件发出者为NULL时，订阅了事件发出者不为NULL的监听者不会收到发布的该ID的Vote事件
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 2，监听者1订阅事件时指定了事件发出者
 3，监听者2订阅相同的事件时未指定事件发出者（为NULL）
 4，发布事件时，没有具体的发出者（为NULL）
 *  - Expect:
 1，监听者1不会收到该事件，监听者2会收到该事件
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::VoteEventTest007()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockVoteEventSink1 sink1;
    MockVoteEventSink1 sink7;

    const int loop = 100;

    EXPECT_CALL(sink1, OnVote(event_category, event_id, actor, &event_ctx_abc, _)).Times(0);
    EXPECT_CALL(sink7, OnVote(event_category, event_id, NULL, &event_ctx_abc, _)).Times(loop).WillRepeatedly(Return(true));

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, NULL, &sink7));

    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, NULL, &sink7, -1));

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, NULL, &sink7));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireVoteEvent(event_category, event_id, NULL, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, NULL, &sink7));

    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveVoteListener(event_category, event_id, NULL, &sink7);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, NULL, &sink7));

    delete actor;
}

/**
 * @brief 有包含关系的Vote事件
 * @details
 *  - Set Up:
 1，事件类别creature是actor的父类别
 *  - Expect:
 1，发布actor的某事件（事件ID为event_common）时，监听creature的该事件的sink也能收到
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::VoteEventTest008()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int parentCategory = fake_event::EVENT_CATEGORY_CREATURE;

    ASSERT_EQ(0, event_center_->SetCategoryRelation(parentCategory, event_category));

    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockVoteEventSink1 sink1;
    MockVoteEventSink1 sink8;

    const int loop = 100;

    EXPECT_CALL(sink1, OnVote(event_category, event_id, actor, &event_ctx_abc, _))
    .Times(loop).WillRepeatedly(Return(true));
    EXPECT_CALL(sink8, OnVote(parentCategory, event_id, actor, &event_ctx_abc, _))
    .Times(loop).WillRepeatedly(Return(true));

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(parentCategory, event_id, actor, &sink8));

    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddVoteListener(parentCategory, event_id, actor, &sink8, -1));

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(parentCategory, event_id, actor, &sink8));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireVoteEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(parentCategory, event_id, actor, &sink8));

    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveVoteListener(parentCategory, event_id, actor, &sink8);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(parentCategory, event_id, actor, &sink8));

    delete actor;
}

/**
 * @brief 有包含关系的Vote事件+NULL监听者
 * @details
 *  - Set Up:
 1，事件类别creature是actor的父类别
 *  - Expect:
 1，发布actor的某事件（事件ID为event_common）时，监听creature的该事件的sink也能收到
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::VoteEventTest009()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int parentCategory = fake_event::EVENT_CATEGORY_CREATURE;

    ASSERT_EQ(0, event_center_->SetCategoryRelation(parentCategory, event_category));

    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockVoteEventSink1 sink1;
    MockVoteEventSink1 sink9;

    const int loop = 100;

    EXPECT_CALL(sink1, OnVote(event_category, event_id, actor, &event_ctx_abc, _))
    .Times(loop).WillRepeatedly(Return(true));
    EXPECT_CALL(sink9, OnVote(parentCategory, event_id, actor, &event_ctx_abc, _))
    .Times(loop).WillRepeatedly(Return(true));

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(parentCategory, event_id, NULL, &sink9));

    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddVoteListener(parentCategory, event_id, NULL, &sink9, -1));

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(parentCategory, event_id, NULL, &sink9));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireVoteEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(parentCategory, event_id, NULL, &sink9));

    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveVoteListener(parentCategory, event_id, NULL, &sink9);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(parentCategory, event_id, NULL, &sink9));

    delete actor;
}

/**
 * @brief 只要有一个监听者投了反对票，则后续的监听者都不会收到事件通知了
 * @details
 *  - Set Up:
 1，两个监听者
 2，第一个监听者投了反对票
 *  - Expect:
 1，第二个监听者收不到投票事件
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::VoteEventTest010()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockVoteEventSink1 sink1;
    MockVoteEventSink1 sink10;

    EXPECT_CALL(sink1, OnVote(event_category, event_id, actor, &event_ctx_abc, 1)).Times(1);
    EXPECT_CALL(sink10, OnVote(event_category, event_id, actor, &event_ctx_abc, -1)).Times(0);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink10));

    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink10, -1));

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink10));

    event_center_->FireVoteEvent(event_category, event_id, actor, &event_ctx_abc);

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink10));

    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveVoteListener(event_category, event_id, actor, &sink10);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink10));

    delete actor;
}

/**
 * @brief Action事件相关接口基本测试
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 *  - Expect:
 1，Action事件的检查、订阅、退订正常
 2，监听sink能收到发布的Action事件
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::ActionEventTest001()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    // 事件现场
    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockActionEventSink1 sink1;

    EXPECT_CALL(sink1, OnAction(event_category, event_id, actor, &event_ctx_abc, 1)).Times(1);
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));

    // 订阅
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));

    // 退订
    event_center_->RemoveActionListener(event_category, event_id, actor, &sink1);
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));

    // 订阅
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));

    // 发布
    event_center_->FireActionEvent(event_category, event_id, actor, &event_ctx_abc);
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));

    // 退订
    event_center_->RemoveActionListener(event_category, event_id, actor, &sink1);
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));

    delete actor;
}

/**
 * @brief Action事件在回调的时候退订自身
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 *  - Expect:
 1，退订之后就收不到后续发布的事件了
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::ActionEventTest002()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockActionEventSink1 sink1;
    MockActionEventSink2 sink2;

    sink2.Delegate(event_center_, actor);

    const int loop = 100;

    EXPECT_CALL(sink1, OnAction(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);
    EXPECT_CALL(sink2, OnAction(event_category, event_id, actor, &event_ctx_abc, 1)).Times(1);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink2));
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink2, -1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink2));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireActionEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink2));

    event_center_->RemoveActionListener(event_category, event_id, actor, &sink1);
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));

    delete actor;
}

/**
 * @brief Action事件在回调的时候先退订自身，然后再订阅自身
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 *  - Expect:
 1，后续发布事件时仍能收到，且次数正确
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::ActionEventTest003()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockActionEventSink1 sink1;
    MockActionEventSink3 sink3;

    sink3.Delegate(event_center_, actor);

    const int loop = 100;

    EXPECT_CALL(sink1, OnAction(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);
    EXPECT_CALL(sink3, OnAction(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink3));
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink3, -1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink3));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireActionEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink3));

    event_center_->RemoveActionListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveActionListener(event_category, event_id, actor, &sink3);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink3));

    delete actor;
}

/**
 * @brief Action事件在回调的时候退订其他的某个事件（回调对象不同）
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 *  - Expect:
 1，后续发布事件时仍能收到，且次数正确
 2，被退订的回调对象收不到后续发布的事件了
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::ActionEventTest004()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockActionEventSink1 sink1;
    MockActionEventSink4 sink4;

    sink4.Delegate(event_center_, actor, &sink1);

    const int loop = 100;

    EXPECT_CALL(sink1, OnAction(event_category, event_id, actor, &event_ctx_abc, 1)).Times(1);

    EXPECT_CALL(sink4, OnAction(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink4));
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink4, -1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink4));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireActionEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink4));

    event_center_->RemoveActionListener(event_category, event_id, actor, &sink4);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink4));

    delete actor;
}

/**
 * @brief Action事件在回调的时候订阅其他的某个事件（回调对象不同）
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 *  - Expect:
 1，后续发布事件时仍能收到，且次数正确
 2，新增加的回调对象可以收到后续发布的事件，且次数正确
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::ActionEventTest005()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockActionEventSink1 sink1;
    MockActionEventSink5 sink5;

    sink5.Delegate(event_center_, actor, &sink1);

    const int loop = 100;

    EXPECT_CALL(sink1, OnAction(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop - 1);
    EXPECT_CALL(sink5, OnAction(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink5));
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink5, -1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink5));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireActionEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink5));

    event_center_->RemoveActionListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveActionListener(event_category, event_id, actor, &sink5);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink5));

    delete actor;
}

/**
 * @brief 事件发出者不为NULL时，订阅了事件发出者为NULL的监听者也能收到发布的该ID的Action事件
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 2，监听者1订阅事件时指定了事件发出者
 3，监听者2订阅相同的事件时未指定事件发出者（为NULL）
 4，发布事件时，有具体的发出者（发出者不为NULL）
 *  - Expect:
 1，监听者1和监听者2都会收到该事件
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::ActionEventTest006()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockActionEventSink1 sink1;
    MockActionEventSink1 sink6;

    const int loop = 100;

    EXPECT_CALL(sink1, OnAction(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_CALL(sink6, OnAction(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, NULL, &sink6));
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, NULL, &sink6, -1));

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, NULL, &sink6));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireActionEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, NULL, &sink6));

    event_center_->RemoveActionListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveActionListener(event_category, event_id, NULL, &sink6);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, NULL, &sink6));

    delete actor;
}

/**
 * @brief 事件发出者为NULL时，订阅了事件发出者不为NULL的监听者不会收到发布的该ID的Action事件
 * @details
 *  - Set Up:
 1，事件类别之间无关系
 2，监听者1订阅事件时指定了事件发出者
 3，监听者2订阅相同的事件时未指定事件发出者（为NULL）
 4，发布事件时，没有具体的发出者（为NULL）
 *  - Expect:
 1，监听者1不会收到该事件，监听者2会收到该事件
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::ActionEventTest007()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockActionEventSink1 sink1;
    MockActionEventSink1 sink7;

    const int loop = 100;

    EXPECT_CALL(sink1, OnAction(event_category, event_id, actor, &event_ctx_abc, -1)).Times(0);
    EXPECT_CALL(sink7, OnAction(event_category, event_id, NULL, &event_ctx_abc, _)).Times(loop);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, NULL, &sink7));

    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, NULL, &sink7, -1));

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, NULL, &sink7));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireActionEvent(event_category, event_id, NULL, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, NULL, &sink7));

    event_center_->RemoveActionListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveActionListener(event_category, event_id, NULL, &sink7);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, NULL, &sink7));

    delete actor;
}

/**
 * @brief 有包含关系的Action事件
 * @details
 *  - Set Up:
 1，事件类别creature是actor的父类别
 *  - Expect:
 1，发布actor的某事件（事件ID为event_common）时，监听creature的该事件的sink也能收到
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::ActionEventTest008()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int parentCategory = fake_event::EVENT_CATEGORY_CREATURE;

    ASSERT_EQ(0, event_center_->SetCategoryRelation(parentCategory, event_category));

    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockActionEventSink1 sink1;
    MockActionEventSink1 sink8;

    const int loop = 100;

    EXPECT_CALL(sink1, OnAction(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_CALL(sink8, OnAction(parentCategory, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(parentCategory, event_id, actor, &sink8));

    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddActionListener(parentCategory, event_id, actor, &sink8, -1));

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(parentCategory, event_id, actor, &sink8));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireActionEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(parentCategory, event_id, actor, &sink8));

    event_center_->RemoveActionListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveActionListener(parentCategory, event_id, actor, &sink8);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(parentCategory, event_id, actor, &sink8));

    delete actor;
}

/**
 * @brief 有包含关系的Action事件+NULL监听者
 * @details
 *  - Set Up:
 1，事件类别creature是actor的父类别
 *  - Expect:
 1，发布actor的某事件（事件ID为event_common）时，监听creature的该事件的sink也能收到
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::ActionEventTest009()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int parentCategory = fake_event::EVENT_CATEGORY_CREATURE;

    ASSERT_EQ(0, event_center_->SetCategoryRelation(parentCategory, event_category));

    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockActionEventSink1 sink1;
    MockActionEventSink1 sink9;

    const int loop = 100;

    EXPECT_CALL(sink1, OnAction(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);
    EXPECT_CALL(sink9, OnAction(parentCategory, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(parentCategory, event_id, NULL, &sink9));

    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddActionListener(parentCategory, event_id, NULL, &sink9, -1));

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(parentCategory, event_id, NULL, &sink9));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireActionEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(parentCategory, event_id, NULL, &sink9));

    event_center_->RemoveActionListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveActionListener(parentCategory, event_id, NULL, &sink9);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(parentCategory, event_id, NULL, &sink9));

    delete actor;
}

/**
 * @brief 既是vote事件监听者又是action事件监听者
 * @details
 *  - Set Up:

 *  - Expect:

 *  - Tear Down:

 * @attention

 */
void EventCenterTest::Test001()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockEventSinkInterface sink1;

    const int loop = 100;

    EXPECT_CALL(sink1, OnVote(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);
    EXPECT_CALL(sink1, OnAction(event_category, event_id, actor, &event_ctx_abc, _)).Times(loop);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));

    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink1, -1));
    EXPECT_EQ(0, event_center_->AddActionListener(event_category, event_id, actor, &sink1, -1));

    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));

    for (int i = 0; i < loop; ++i)
    {
        event_center_->FireVoteEvent(event_category, event_id, actor, &event_ctx_abc);
        event_center_->FireActionEvent(event_category, event_id, actor, &event_ctx_abc);
    }

    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_TRUE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));

    event_center_->RemoveActionListener(event_category, event_id, actor, &sink1);
    event_center_->RemoveActionListener(event_category, event_id, actor, &sink1);

    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));
    EXPECT_FALSE(event_center_->ActionListenerExist(event_category, event_id, actor, &sink1));

    delete actor;
}

/**
 * @brief 助手类测试
 * @details
 *  - Set Up:
 1，使用助手类添加一个vote event_sink
 2，使用助手类添加一个action event_sink
 3，不手动移除这两个sink
 *  - Expect:
 1，助手类在退出时会自动移除所有的sink
 *  - Tear Down:

 * @attention

 */
void EventCenterTest::Test002()
{
    NiceMock<MockEventCenterInterface> mock_event_center;

    EXPECT_CALL(mock_event_center, AddVoteListener(_, _, _, _, _)).Times(1);
    EXPECT_CALL(mock_event_center, RemoveVoteListener(_, _, _, _)).Times(1);

    EXPECT_CALL(mock_event_center, AddActionListener(_, _, _, _, _)).Times(1);
    EXPECT_CALL(mock_event_center, RemoveActionListener(_, _, _, _)).Times(1);

    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    MockEventSinkInterface sink1;

    {
        EventCenterUtil event_center_util;
        event_center_util.SetEventCenter(&mock_event_center);

        EXPECT_EQ(0, event_center_util.AddVoteEventListener(event_category, event_id, actor, &sink1, -1));
        EXPECT_EQ(0, event_center_util.AddActionEventListener(event_category, event_id, actor, &sink1, -1));
    }

    delete actor;
}

void EventCenterTest::RemoveTest001()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::ActorInterface* actor = new fake_event::Actor();
    ASSERT_TRUE(actor != NULL);

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = actor;

    MockVoteEventSink2 sink2;

    sink2.Delegate(event_center_, actor);

    EXPECT_CALL(sink2, OnVote(event_category, event_id, actor, &event_ctx_abc, 1)).Times(1);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink2));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, actor, &sink2, -1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink2));

    event_center_->FireVoteEvent(event_category, event_id, actor, &event_ctx_abc);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, actor, &sink2));

    delete actor;
}

void EventCenterTest::RemoveTest002()
{
    int event_category = fake_event::EVENT_CATEGORY_ACTOR;
    int event_id = fake_event::EVENT_ID_ABC;

    fake_event::EVENT_CTX_ABC event_ctx_abc;
    event_ctx_abc.actor = NULL;

    MockVoteEventSink2 sink2;

    sink2.Delegate(event_center_, NULL);

    EXPECT_CALL(sink2, OnVote(event_category, event_id, NULL, &event_ctx_abc, 1)).Times(1);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, NULL, &sink2));
    EXPECT_EQ(0, event_center_->AddVoteListener(event_category, event_id, NULL, &sink2, -1));
    EXPECT_TRUE(event_center_->VoteListenerExist(event_category, event_id, NULL, &sink2));

    event_center_->FireVoteEvent(event_category, event_id, NULL, &event_ctx_abc);

    EXPECT_FALSE(event_center_->VoteListenerExist(event_category, event_id, NULL, &sink2));
}

struct EventCenterPair
{
    ModuleLoader loader;
    EventCenterInterface* event_center;
};

void EventCenterTest::LoadSoNumTest()
{
    typedef std::vector<EventCenterPair> EventCenterVec;
    EventCenterVec event_center_vec;

    const int n = 100000;
    event_center_vec.resize(n);

    for (int i = 0; i < n; ++i)
    {
        if (event_center_vec[i].loader.Load("../libevent_center.so") != 0)
        {
            FAIL() << event_center_vec[i].loader.GetLastErrMsg();
        }

        event_center_vec[i].event_center = (EventCenterInterface*) event_center_vec[i].loader.GetModuleInterface();
        if (NULL == event_center_vec[i].event_center)
        {
            FAIL() << event_center_vec[i].loader.GetLastErrMsg();
        }
    }

    for (EventCenterVec::iterator it = event_center_vec.begin(); it != event_center_vec.end(); ++it)
    {
        if (it->event_center != NULL)
        {
            it->event_center->Release();
        }

        it->loader.Unload();
    }
}

ADD_TEST_F(EventCenterTest, VoteEventTest001);
ADD_TEST_F(EventCenterTest, VoteEventTest002);
ADD_TEST_F(EventCenterTest, VoteEventTest003);
ADD_TEST_F(EventCenterTest, VoteEventTest004);
ADD_TEST_F(EventCenterTest, VoteEventTest005);
ADD_TEST_F(EventCenterTest, VoteEventTest006);
ADD_TEST_F(EventCenterTest, VoteEventTest007);
ADD_TEST_F(EventCenterTest, VoteEventTest008);
ADD_TEST_F(EventCenterTest, VoteEventTest009);
ADD_TEST_F(EventCenterTest, VoteEventTest010);
ADD_TEST_F(EventCenterTest, ActionEventTest001);
ADD_TEST_F(EventCenterTest, ActionEventTest002);
ADD_TEST_F(EventCenterTest, ActionEventTest003);
ADD_TEST_F(EventCenterTest, ActionEventTest004);
ADD_TEST_F(EventCenterTest, ActionEventTest005);
ADD_TEST_F(EventCenterTest, ActionEventTest006);
ADD_TEST_F(EventCenterTest, ActionEventTest007);
ADD_TEST_F(EventCenterTest, ActionEventTest008);
ADD_TEST_F(EventCenterTest, ActionEventTest009);
ADD_TEST_F(EventCenterTest, Test001);
ADD_TEST_F(EventCenterTest, Test002);
ADD_TEST_F(EventCenterTest, RemoveTest001);
ADD_TEST_F(EventCenterTest, RemoveTest002);
