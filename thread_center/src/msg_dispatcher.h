#ifndef THREAD_CENTER_SRC_MSG_DISPATCHER_H_
#define THREAD_CENTER_SRC_MSG_DISPATCHER_H_

#include "hash_container.h"
#include "msg_handler_interface.h"

namespace thread_center
{
class MsgDispatcher : public base::MsgDispatcherInterface
{
public:
    MsgDispatcher();
    virtual ~MsgDispatcher();

    ///////////////////////// MsgDispatcherInterface /////////////////////////
    int AttachMsgHandler(MsgId msg_id, base::MsgHandlerInterface* msg_handler) override;
    void DetachMsgHandler(MsgId msg_id) override;
    int DispatchMsg(const ConnGuid* conn_guid, const MsgHead& msg_head, const void* msg_body,
                    size_t msg_body_len) override;

protected:
    typedef __hash_map<MsgId, base::MsgHandlerInterface*> MsgHandlerMap;
    MsgHandlerMap msg_handler_map_;
};
}

#endif // THREAD_CENTER_SRC_MSG_DISPATCHER_H_
