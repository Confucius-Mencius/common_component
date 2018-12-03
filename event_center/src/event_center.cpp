#include "event_center.h"
#include "version.h"

namespace event_center
{
EventCenter::EventCenter() : last_err_msg_(), vote_event_category_table_(), action_event_category_table_(),
                             event_category_relation_vec_()
{
}

EventCenter::~EventCenter()
{
}

const char* EventCenter::GetVersion() const
{
    return EVENT_CENTER_EVENT_CENTER_VERSION;
}

const char* EventCenter::GetLastErrMsg() const
{
    return last_err_msg_.What();
}

void EventCenter::Release()
{
    delete this;
}

int EventCenter::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    const EventCenterCtx* event_center_ctx = (const EventCenterCtx*) ctx;
    const int ncategories = event_center_ctx->ncategories;
    if (ncategories <= 0)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "invalid event category count: " << ncategories << ", must > 0");
        return -1;
    }

    event_category_relation_vec_.resize((size_t) ncategories);

    for (int i = 0; i < ncategories; ++i)
    {
        event_category_relation_vec_[i] = EVENT_CATEGORY_ROOT;
    }

    return 0;
}

void EventCenter::Finalize()
{
    vote_event_category_table_.clear();
    action_event_category_table_.clear();
    event_category_relation_vec_.clear();
}

int EventCenter::Activate()
{
    return 0;
}

void EventCenter::Freeze()
{
}

int EventCenter::SetCategoryRelation(int parent, int child)
{
    const int event_category_count = (int) event_category_relation_vec_.size();
    if (parent < 0 || parent >= event_category_count || child < 0 || child >= event_category_count)
    {
        return -1;
    }

    if (event_category_relation_vec_[child] != EVENT_CATEGORY_ROOT)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "child: " << child << " already have parent: "
            << event_category_relation_vec_[child]);
        return -1;
    }

    event_category_relation_vec_[child] = parent;
    return 0;
}

bool EventCenter::VoteListenerExist(int event_category, int event_id, void* event_provider,
                                    VoteEventSinkInterface* event_sink) const
{
    if (NULL == event_sink)
    {
        return false;
    }

    return EventListenerExist(event_category, event_id, event_provider, event_sink, false);
}

int EventCenter::AddVoteListener(int event_category, int event_id, void* event_provider,
                                 VoteEventSinkInterface* event_sink, int total_times)
{
    if (NULL == event_sink)
    {
        return -1;
    }

    return AddEventListener(event_category, event_id, event_provider, event_sink, total_times, false);
}

void EventCenter::RemoveVoteListener(int event_category, int event_id, void* event_provider,
                                     VoteEventSinkInterface* event_sink)
{
    if (NULL == event_sink)
    {
        return;
    }

    RemoveEventListener(event_category, event_id, event_provider, event_sink, false);
}

bool EventCenter::FireVoteEvent(int event_category, int event_id, void* event_provider, void* event_ctx)
{
    return FireEvent(event_category, event_id, event_provider, event_ctx, false);
}

bool EventCenter::ActionListenerExist(int event_category, int event_id, void* event_provider,
                                      ActionEventSinkInterface* event_sink) const
{
    if (NULL == event_sink)
    {
        return false;
    }

    return EventListenerExist(event_category, event_id, event_provider, event_sink, true);
}

int EventCenter::AddActionListener(int event_category, int event_id, void* event_provider,
                                   ActionEventSinkInterface* event_sink, int total_times)
{
    if (NULL == event_sink)
    {
        return -1;
    }

    return AddEventListener(event_category, event_id, event_provider, event_sink, total_times, true);
}

void EventCenter::RemoveActionListener(int event_category, int event_id, void* event_provider,
                                       ActionEventSinkInterface* event_sink)
{
    if (NULL == event_sink)
    {
        return;
    }

    RemoveEventListener(event_category, event_id, event_provider, event_sink, true);
}

void EventCenter::FireActionEvent(int event_category, int event_id, void* event_provider, void* event_ctx)
{
    FireEvent(event_category, event_id, event_provider, event_ctx, true);
}

EventCenter::EventSinkMap* EventCenter::GetEventSinkMap(int event_category, int event_id, void* event_provider,
                                                        bool is_action_event) const
{
    const EventCategoryMap& event_category_map = is_action_event ? action_event_category_table_
                                                                 : vote_event_category_table_;
    EventCategoryMap::const_iterator it_event_category = event_category_map.find(event_category);
    if (it_event_category == event_category_map.end())
    {
        return NULL;
    }

    const EventIDMap& event_id_map = it_event_category->second.event_id_map;
    EventIDMap::const_iterator it_event_id = event_id_map.find(event_id);
    if (it_event_id == event_id_map.end())
    {
        return NULL;
    }

    const EventProviderMap& event_provider_map = it_event_id->second.event_provider_map;
    EventProviderMap::const_iterator it_event_provider = event_provider_map.find(event_provider);
    if (it_event_provider == event_provider_map.end())
    {
        return NULL;
    }

    return const_cast<EventSinkMap*>(&(it_event_provider->second.event_sink_map));
}

bool EventCenter::EventListenerExist(int event_category, int event_id, void* event_provider, void* event_sink,
                                     bool is_action_event) const
{
    EventSinkMap* event_sink_map = GetEventSinkMap(event_category, event_id, event_provider, is_action_event);
    if (NULL == event_sink_map)
    {
        return false;
    }

    EventSinkMap::const_iterator it = event_sink_map->find(event_sink);

    if (it == event_sink_map->end())
    {
        return false;
    }

    return !(it->second.removed);
}

int EventCenter::AddEventListener(int event_category, int event_id, void* event_provider, void* event_sink,
                                  int total_times, bool is_action_event)
{
    EventCategoryMap& event_category_map = is_action_event ? action_event_category_table_ : vote_event_category_table_;
    EventIDMap& event_id_map = event_category_map[event_category].event_id_map;
    EventProviderMap& event_provider_map = event_id_map[event_id].event_provider_map;
    EventSinkMap* event_sink_map = &(event_provider_map[event_provider].event_sink_map);

    EventSinkMap::iterator it_event_sink = event_sink_map->find(event_sink);
    if (it_event_sink != event_sink_map->end())
    {
        if (!it_event_sink->second.removed)
        {
            SET_LAST_ERR_MSG(&last_err_msg_, "event listener already exist, event category: " << event_category
                << ", event id: " << event_id << ", event provider: " << event_provider
                << ", event sink: " << event_sink);
            return -1;
        }

        if (is_action_event)
        {
            it_event_sink->second.Create(event_category, event_id, event_provider,
                                         (ActionEventSinkInterface*) event_sink, total_times);
        }
        else
        {
            it_event_sink->second.Create(event_category, event_id, event_provider,
                                         (VoteEventSinkInterface*) event_sink, total_times);
        }

        return 0;
    }

    EventListener event_listener;

    if (is_action_event)
    {
        event_listener.Create(event_category, event_id, event_provider,
                              (ActionEventSinkInterface*) event_sink, total_times);
    }
    else
    {
        event_listener.Create(event_category, event_id, event_provider,
                              (VoteEventSinkInterface*) event_sink, total_times);
    }

    if (!event_sink_map->insert(EventSinkMap::value_type(event_sink, event_listener)).second)
    {
        SET_LAST_ERR_MSG(&last_err_msg_, "failed to insert to map, event sink: " << event_sink);
        return -1;
    }

    return 0;
}

void EventCenter::RemoveEventListener(int event_category, int event_id, void* event_provider, void* event_sink,
                                      bool is_action_event)
{
    EventSinkMap* event_sink_map = GetEventSinkMap(event_category, event_id, event_provider, is_action_event);
    if (NULL == event_sink_map)
    {
        return;
    }

    EventSinkMap::iterator event_sink_it = event_sink_map->find(event_sink);
    if (event_sink_it == event_sink_map->end())
    {
        return;
    }

    EventListener& event_listener = event_sink_it->second;
    if (event_listener.calling)
    {
        event_listener.removed = true; // 正在调用不能删除, 先设置标记
    }
    else
    {
        event_sink_map->erase(event_sink_it); // 没有被调用, 直接删除
    }
}

bool EventCenter::FireEvent(int event_category, int event_id, void* event_provider, void* event_ctx,
                            bool is_action_event)
{
    void* provider = event_provider;

    do
    {
        EventSinkMap* event_sink_map = GetEventSinkMap(event_category, event_id, provider, is_action_event);
        if (NULL == event_sink_map)
        {
            if (provider != NULL)
            {
                provider = NULL;
                continue;
            }

            break;
        }

        for (EventSinkMap::iterator it = event_sink_map->begin(); it != event_sink_map->end();)
        {
            EventListener& event_listener = it->second;

            event_listener.calling = true;
            const int times = ++event_listener.times;
            bool ret = event_listener.OnEvent(event_category, event_id, event_provider, event_ctx, times);
            event_listener.calling = false;

            if (event_listener.removed || (event_listener.total_times > 0 && times >= event_listener.total_times))
            {
                event_sink_map->erase(it++);
            }
            else
            {
                ++it;
            }

            if (!is_action_event && !ret)
            {
                return false;
            }
        }

        if (provider != NULL)
        {
            provider = NULL;
            continue;
        }

        break;
    } while (true);

    const int parent = event_category_relation_vec_[event_category];
    if (EVENT_CATEGORY_ROOT == parent)
    {
        return true;
    }

    return FireEvent(parent, event_id, event_provider, event_ctx, is_action_event);
}
}
