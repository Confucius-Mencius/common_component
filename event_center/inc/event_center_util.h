#ifndef EVENT_CENTER_INC_EVENT_UTIL_H_
#define EVENT_CENTER_INC_EVENT_UTIL_H_

#include <set>
#include "event_center_interface.h"

/**
 * @brief 事件助手类
 */
class EventCenterUtil
{
public:
    EventCenterUtil()
    {
        event_center_ = NULL;
    }

    ~EventCenterUtil()
    {
        if (NULL == event_center_)
        {
            return;
        }

        for (EventListenerSet::const_iterator it = vote_event_listener_set_.begin();
             it != vote_event_listener_set_.end(); ++it)
        {
            const EventListener& eventListener = *it;
            event_center_->RemoveVoteListener(eventListener.event_category, eventListener.event_id,
                                              eventListener.event_provider,
                                              (VoteEventSinkInterface*) eventListener.event_sink);
        }

        for (EventListenerSet::const_iterator it = action_event_listener_set_.begin();
             it != action_event_listener_set_.end(); ++it)
        {
            const EventListener& eventListener = *it;
            event_center_->RemoveActionListener(eventListener.event_category, eventListener.event_id,
                                                eventListener.event_provider,
                                                (ActionEventSinkInterface*) eventListener.event_sink);
        }

        vote_event_listener_set_.clear();
        action_event_listener_set_.clear();
    }

    void SetEventCenter(EventCenterInterface* event_center)
    {
        event_center_ = event_center;
    }

    int AddVoteEventListener(int event_category, int event_id, void* event_provider, VoteEventSinkInterface* event_sink,
                             int total_times)
    {
        if (NULL == event_center_)
        {
            return -1;
        }

        int ret = event_center_->AddVoteListener(event_category, event_id, event_provider, event_sink, total_times);
        if (ret != 0)
        {
            return ret;
        }

        EventListener event_listener;
        event_listener.Create(event_category, event_id, event_provider, event_sink);

        if (!vote_event_listener_set_.insert(event_listener).second)
        {
            event_center_->RemoveVoteListener(event_category, event_id, event_provider, event_sink);
            return -1;
        }

        return 0;
    }

    void RemoveVoteEventListener(int event_category, int event_id, void* event_provider,
                                 VoteEventSinkInterface* event_sink)
    {
        if (NULL == event_center_)
        {
            return;
        }

        EventListener event_listener;
        event_listener.Create(event_category, event_id, event_provider, event_sink);

        EventListenerSet::const_iterator it = vote_event_listener_set_.find(event_listener);
        if (it != vote_event_listener_set_.end())
        {
            event_center_->RemoveVoteListener(event_listener.event_category, event_listener.event_id,
                                              event_listener.event_provider,
                                              (VoteEventSinkInterface*) event_listener.event_sink);
            vote_event_listener_set_.erase(it);
        }
    }

    int AddActionEventListener(int event_category, int event_id, void* event_provider,
                               ActionEventSinkInterface* event_sink, int total_times)
    {
        if (NULL == event_center_)
        {
            return -1;
        }

        int ret = event_center_->AddActionListener(event_category, event_id, event_provider, event_sink, total_times);
        if (ret != 0)
        {
            return ret;
        }

        EventListener event_listener;
        event_listener.Create(event_category, event_id, event_provider, event_sink);

        if (!action_event_listener_set_.insert(event_listener).second)
        {
            event_center_->RemoveActionListener(event_category, event_id, event_provider, event_sink);
            return -1;
        }

        return 0;
    }

    void RemoveActionEventListener(int event_category, int event_id, void* event_provider,
                                   ActionEventSinkInterface* event_sink)
    {
        if (NULL == event_center_)
        {
            return;
        }

        EventListener event_listener;
        event_listener.Create(event_category, event_id, event_provider, event_sink);

        EventListenerSet::iterator it = action_event_listener_set_.find(event_listener);
        if (it != action_event_listener_set_.end())
        {
            event_center_->RemoveActionListener(event_listener.event_category, event_listener.event_id,
                                                event_listener.event_provider,
                                                (ActionEventSinkInterface*) event_listener.event_sink);
            action_event_listener_set_.erase(it);
        }
    }

private:
    struct EventListener
    {
        int event_category;
        int event_id;
        void* event_provider;
        void* event_sink;

        void Create(int event_categroy, int event_id, void* event_provider, void* event_sink)
        {
            this->event_category = event_categroy;
            this->event_id = event_id;
            this->event_provider = event_provider;
            this->event_sink = event_sink;
        }

        bool operator<(const EventListener& rhs) const
        {
            if (event_category != rhs.event_category)
            {
                return event_category < rhs.event_category;
            }
            else if (event_id != rhs.event_id)
            {
                return event_id < rhs.event_id;
            }
            else if (event_provider != rhs.event_provider)
            {
                return event_provider < rhs.event_provider;
            }
            else
            {
                return event_sink < rhs.event_sink;
            }
        }
    };

    typedef std::set<EventListener> EventListenerSet;

    EventCenterInterface* event_center_;
    EventListenerSet vote_event_listener_set_;
    EventListenerSet action_event_listener_set_;
};

#endif // EVENT_CENTER_INC_EVENT_UTIL_H_
