#ifndef PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_MSG_DISPATCHER_H_
#define PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_MSG_DISPATCHER_H_

#include <map>
#include "proto_msg_handler_interface.h"

namespace tcp
{
namespace proto
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
}

#endif // PROTO_TCP_THREADS_SRC_RAW_TCP_COMMON_LOGIC_MSG_DISPATCHER_H_
