#ifndef EVENT_CENTER_SRC_EVENT_CENTER_H_
#define EVENT_CENTER_SRC_EVENT_CENTER_H_

#include <map>
#include <vector>
#include "event_center_define.h"
#include "last_err_msg.h"

namespace event_center
{
class EventCenter : public EventCenterInterface
{
public:
    EventCenter();
    virtual ~EventCenter();

    ///////////////////////// ModuleInterface /////////////////////////
    virtual const char* GetVersion() const;
    virtual const char* GetLastErrMsg() const;
    virtual void Release();
    virtual int Initialize(const void* ctx);
    virtual void Finalize();
    virtual int Activate();
    virtual void Freeze();

    ///////////////////////// EventCenterInterface /////////////////////////
    virtual int SetCategoryRelation(int parent, int child);
    virtual bool VoteListenerExist(int event_category, int event_id, void* event_provider,
                                   VoteEventSinkInterface* event_sink) const;
    virtual int AddVoteListener(int event_category, int event_id, void* event_provider,
                                VoteEventSinkInterface* event_sink, int total_times);
    virtual void RemoveVoteListener(int event_category, int event_id, void* event_provider,
                                    VoteEventSinkInterface* event_sink);
    virtual bool FireVoteEvent(int event_category, int event_id, void* event_provider, void* event_ctx);

    virtual bool ActionListenerExist(int event_category, int event_id, void* event_provider,
                                     ActionEventSinkInterface* event_sink) const;
    virtual int AddActionListener(int event_category, int event_id, void* event_provider,
                                  ActionEventSinkInterface* event_sink, int total_times);
    virtual void RemoveActionListener(int event_category, int event_id, void* event_provider,
                                      ActionEventSinkInterface* event_sink);
    virtual void FireActionEvent(int event_category, int event_id, void* event_provider, void* event_ctx);

private:
    static const int EVENT_CATEGORY_ROOT = -1;

private:
    // event_sink -> event
    typedef std::map<void*, EventListener> EventSinkMap;
    struct EventSinkMapWrapper
    {
        EventSinkMap event_sink_map;
    };

    // event_provider -> event_sink map
    typedef std::map<void*, EventSinkMapWrapper> EventProviderMap;
    struct EventProviderMapWrapper
    {
        EventProviderMap event_provider_map;
    };

    // event event_id -> event_provider map
    typedef std::map<int, EventProviderMapWrapper> EventIDMap;
    struct EventIDMapWrapper
    {
        EventIDMap event_id_map;
    };

    // event category -> event event_id map
    typedef std::map<int, EventIDMapWrapper> EventCategoryMap;

private:
    EventSinkMap* GetEventSinkMap(int event_category, int event_id, void* event_provider, bool is_action_event) const;
    bool EventListenerExist(int event_category, int event_id, void* event_provider, void* event_sink,
                            bool is_action_event) const;
    int AddEventListener(int event_category, int event_id, void* event_provider, void* event_sink, int total_times,
                         bool is_action_event);
    void RemoveEventListener(int event_category, int event_id, void* event_provider, void* event_sink,
                             bool is_action_event);
    bool FireEvent(int event_category, int event_id, void* event_provider, void* event_ctx, bool is_action_event);

private:
    LastErrMsg last_err_msg_;

    EventCategoryMap vote_event_category_table_;
    EventCategoryMap action_event_category_table_;

    typedef std::vector<int> EventCategoryRelationVec;
    EventCategoryRelationVec event_category_relation_vec_;
};
}

#endif // EVENT_CENTER_SRC_EVENT_CENTER_H_
