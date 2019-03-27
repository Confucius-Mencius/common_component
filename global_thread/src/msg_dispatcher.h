#ifndef GLOBAL_THREAD_SRC_MSG_DISPATCHER_H_
#define GLOBAL_THREAD_SRC_MSG_DISPATCHER_H_

#include <map>
#include "global_msg_handler_interface.h"

namespace global
{
class MsgDispatcher : public ::proto::MsgDispatcherInterface
{
public:
    MsgDispatcher();
    virtual ~MsgDispatcher();

    ///////////////////////// MsgDispatcherInterface /////////////////////////
    int AttachMsgHandler(::proto::MsgID msg_id, ::proto::MsgHandlerInterface* msg_handler) override;
    void DetachMsgHandler(::proto::MsgID msg_id) override;
    int DispatchMsg(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body,
                    size_t msg_body_len) override;

protected:
    typedef std::map<::proto::MsgID, ::proto::MsgHandlerInterface*> MsgHandlerMap;
    MsgHandlerMap msg_handler_map_;
};
}

#endif // GLOBAL_THREAD_SRC_MSG_DISPATCHER_H_
