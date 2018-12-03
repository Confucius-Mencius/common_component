#ifndef EVENT_CENTER_SRC_EVENT_CENTER_DEFINE_H_
#define EVENT_CENTER_SRC_EVENT_CENTER_DEFINE_H_

#include <stddef.h>
#include "event_center_interface.h"

struct EventListener
{
    int event_category;
    int event_id;
    void* event_provider;
    void* event_sink;
    int total_times;
    int times;
    bool is_action_event;
    bool calling;
    bool removed;

    void Create(int event_category, int event_id, void* event_provider, VoteEventSinkInterface* event_sink,
                int total_times)
    {
        this->event_category = event_category;
        this->event_id = event_id;
        this->event_provider = event_provider;
        this->event_sink = event_sink;
        this->total_times = total_times;
        this->times = 0;
        is_action_event = false;
        calling = false;
        removed = false;
    }

    void Create(int event_category, int event_id, void* event_provider, ActionEventSinkInterface* event_sink,
                int total_times)
    {
        this->event_category = event_category;
        this->event_id = event_id;
        this->event_provider = event_provider;
        this->event_sink = event_sink;
        this->total_times = total_times;
        this->times = 0;
        is_action_event = true;
        calling = false;
        removed = false;
    }

    bool OnEvent(int event_category, int event_id, void* event_provider, void* event_ctx, int times)
    {
        if (is_action_event)
        {
            ActionEventSinkInterface* event_sink = (ActionEventSinkInterface*) this->event_sink;
            if (event_sink != NULL)
            {
                event_sink->OnAction(event_category, event_id, event_provider, event_ctx, times);
                return true;
            }
        }
        else
        {
            VoteEventSinkInterface* event_sink = (VoteEventSinkInterface*) this->event_sink;
            if (event_sink != NULL)
            {
                return event_sink->OnVote(event_category, event_id, event_provider, event_ctx, times);
            }
        }

        return false;
    }
};

#endif // EVENT_CENTER_SRC_EVENT_CENTER_DEFINE_H_
