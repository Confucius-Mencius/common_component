#ifndef APP_FRAME_INC_HTTP_PROTOBUF_MSG_UTIL_H_
#define APP_FRAME_INC_HTTP_PROTOBUF_MSG_UTIL_H_

#include "http_scheduler_interface.h"
#include "protobuf_msg_util.h"

inline TransId SendToWorkThread(http::SchedulerInterface* scheduler, const ConnGuid* conn_guid,
                                const MsgHead& msg_head, const google::protobuf::Message* protobuf_msg,
                                int work_thread_idx, const base::AsyncCtx* async_ctx)
{
    char* msg_body = NULL;
    size_t msg_body_len = 0;

    if (SerializeProtobufMsg(&msg_body, msg_body_len, protobuf_msg) != 0)
    {
        return INVALID_TRANS_ID;
    }

    TransId trans_id = scheduler->SendToWorkThread(conn_guid, msg_head, msg_body, msg_body_len, work_thread_idx,
                       async_ctx);
    FreeProtobufMsgBuf(&msg_body);

    return trans_id;
}

inline TransId SendToGlobalThread(http::SchedulerInterface* scheduler, const ConnGuid* conn_guid,
                                  const MsgHead& msg_head, const google::protobuf::Message* protobuf_msg,
                                  const base::AsyncCtx* async_ctx)
{
    char* msg_body = NULL;
    size_t msg_body_len = 0;

    if (SerializeProtobufMsg(&msg_body, msg_body_len, protobuf_msg) != 0)
    {
        return INVALID_TRANS_ID;
    }

    TransId trans_id = scheduler->SendToGlobalThread(conn_guid, msg_head, msg_body, msg_body_len, async_ctx);
    FreeProtobufMsgBuf(&msg_body);

    return trans_id;
}

#endif // APP_FRAME_INC_HTTP_PROTOBUF_MSG_UTIL_H_
