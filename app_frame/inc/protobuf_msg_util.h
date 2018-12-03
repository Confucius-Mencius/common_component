#ifndef APP_FRAME_INC_PROTOBUF_MSG_UTIL_H_
#define APP_FRAME_INC_PROTOBUF_MSG_UTIL_H_

#include <google/protobuf/message.h>
#include "scheduler_interface.h"

inline int ParseProtobufMsg(google::protobuf::Message* protobuf_msg, const void* msg, size_t msg_len)
{
    if (NULL == protobuf_msg || NULL == msg || 0 == msg_len)
    {
        return -1;
    }

    return protobuf_msg->ParseFromArray(msg, msg_len) ? 0 : -1;
}

// 返回的msg指针需要调用FreeProtobufMsgBuf释放
inline int SerializeProtobufMsg(char** msg, size_t& msg_len, const google::protobuf::Message* protobuf_msg)
{
    if (NULL == protobuf_msg)
    {
        return 0;
    }

    msg_len = protobuf_msg->ByteSize();
    char* buf = new char[msg_len];
    if (NULL == buf)
    {
        return -1;
    }

    if (!protobuf_msg->SerializeToArray(buf, msg_len))
    {
        delete[] buf;
        buf = NULL;
        return -1;
    }

    *msg = buf;
    return 0;
}

inline void FreeProtobufMsgBuf(char** msg)
{
    if (*msg != NULL)
    {
        delete[] (*msg);
        *msg = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////////
// peer
inline TransId SendToServer(base::SchedulerInterface* scheduler, const Peer& peer, const MsgHead& msg_head,
                            const google::protobuf::Message* protobuf_msg, base::AsyncCtx* async_ctx)
{
    char* msg_body = NULL;
    size_t msg_body_len = 0;

    if (SerializeProtobufMsg(&msg_body, msg_body_len, protobuf_msg) != 0)
    {
        return INVALID_TRANS_ID;
    }

    TransId trans_id = scheduler->SendToServer(peer, msg_head, msg_body, msg_body_len, async_ctx);
    FreeProtobufMsgBuf(&msg_body);

    return trans_id;
}

#endif // APP_FRAME_INC_PROTOBUF_MSG_UTIL_H_
