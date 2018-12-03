#ifndef EVENT_CENTER_TEST_FAKE_EVENT_DEFINE_H_
#define EVENT_CENTER_TEST_FAKE_EVENT_DEFINE_H_

#include "event_center_interface.h"

namespace fake_event
{
enum
{
    EVENT_CATEGORY_MIN = 0,
    EVENT_CATEGORY_CREATURE = EVENT_CATEGORY_MIN,
    EVENT_CATEGORY_ACTOR,
    EVENT_CATEGORY_MONSTER,

    // ...

    EVENT_CATEGORY_MAX
};

enum
{
    EVENT_ID_MIN = 0,
    EVENT_ID_ABC = EVENT_ID_MIN,
    EVENT_ID_XYZ,

    // ...

    EVENT_ID_MAX,
};

class CreatureInterface
{
};

class ActorInterface : public CreatureInterface
{
};

class Actor : public ActorInterface
{
};

// category: EVENT_CATEGORY_ACTOR
// event id: EVENT_ID_ABC
// event_provider: ActorInterface*
// event type: vote, action
struct EVENT_CTX_ABC
{
    ActorInterface* actor;
};
}

#endif // EVENT_CENTER_TEST_FAKE_EVENT_DEFINE_H_
