#ifndef APP_FRAME_INC_WORK_PROTOBUF_MSG_UTIL_H_
#define APP_FRAME_INC_WORK_PROTOBUF_MSG_UTIL_H_

#include "work_scheduler_interface.h"
#include "protobuf_msg_util.h"

inline int SendToWorkThread(work::SchedulerInterface* scheduler, const ConnGuid* conn_guid,
                            const MsgHead& msg_head, const google::protobuf::Message* protobuf_msg, int work_thread_idx)
{
    char* msg_body = NULL;
    size_t msg_body_len = 0;

    if (SerializeProtobufMsg(&msg_body, msg_body_len, protobuf_msg) != 0)
    {
        return -1;
    }

    int ret = scheduler->SendToWorkThread(conn_guid, msg_head, msg_body, msg_body_len, work_thread_idx);
    FreeProtobufMsgBuf(&msg_body);

    return ret;
}

inline int SendToBurdenThread(work::SchedulerInterface* scheduler, const ConnGuid* conn_guid,
                              const MsgHead& msg_head, const google::protobuf::Message* protobuf_msg,
                              int burden_thread_idx)
{
    char* msg_body = NULL;
    size_t msg_body_len = 0;

    if (SerializeProtobufMsg(&msg_body, msg_body_len, protobuf_msg) != 0)
    {
        return -1;
    }

    int ret = scheduler->SendToBurdenThread(conn_guid, msg_head, msg_body, msg_body_len, burden_thread_idx);
    FreeProtobufMsgBuf(&msg_body);

    return ret;
}

inline int SendToGlobalThread(work::SchedulerInterface* scheduler, const ConnGuid* conn_guid,
                              const MsgHead& msg_head, const google::protobuf::Message* protobuf_msg)
{
    char* msg_body = NULL;
    size_t msg_body_len = 0;

    if (SerializeProtobufMsg(&msg_body, msg_body_len, protobuf_msg) != 0)
    {
        return -1;
    }

    int ret = scheduler->SendToGlobalThread(conn_guid, msg_head, msg_body, msg_body_len);
    FreeProtobufMsgBuf(&msg_body);

    return ret;
}

inline int SendToTcpClient(work::SchedulerInterface* scheduler, const ConnGuid* conn_guid,
                           const MsgHead& msg_head, const google::protobuf::Message* protobuf_msg)
{
    char* msg_body = NULL;
    size_t msg_body_len = 0;

    if (SerializeProtobufMsg(&msg_body, msg_body_len, protobuf_msg) != 0)
    {
        return -1;
    }

    int ret = scheduler->SendToTcpClient(conn_guid, msg_head, msg_body, msg_body_len);
    FreeProtobufMsgBuf(&msg_body);

    return ret;
}

inline int SendToUdpClient(work::SchedulerInterface* scheduler, const ConnGuid* conn_guid,
                           const MsgHead& msg_head, const google::protobuf::Message* protobuf_msg)
{
    char* msg_body = NULL;
    size_t msg_body_len = 0;

    if (SerializeProtobufMsg(&msg_body, msg_body_len, protobuf_msg) != 0)
    {
        return -1;
    }

    int ret = scheduler->SendToUdpClient(conn_guid, msg_head, msg_body, msg_body_len);
    FreeProtobufMsgBuf(&msg_body);

    return ret;
}

inline int SendToTcpThread(work::SchedulerInterface* scheduler, const ConnGuid* conn_guid,
                           const MsgHead& msg_head, const google::protobuf::Message* protobuf_msg, int tcp_thread_idx)
{
    char* msg_body = NULL;
    size_t msg_body_len = 0;

    if (SerializeProtobufMsg(&msg_body, msg_body_len, protobuf_msg) != 0)
    {
        return -1;
    }

    int ret = scheduler->SendToTcpThread(conn_guid, msg_head, msg_body, msg_body_len, tcp_thread_idx);
    FreeProtobufMsgBuf(&msg_body);

    return ret;
}

inline int SendToHttpThread(work::SchedulerInterface* scheduler, const ConnGuid* conn_guid,
                            const MsgHead& msg_head, const google::protobuf::Message* protobuf_msg, int http_thread_idx)
{
    char* msg_body = NULL;
    size_t msg_body_len = 0;

    if (SerializeProtobufMsg(&msg_body, msg_body_len, protobuf_msg) != 0)
    {
        return -1;
    }

    int ret = scheduler->SendToHttpThread(conn_guid, msg_head, msg_body, msg_body_len, http_thread_idx);
    FreeProtobufMsgBuf(&msg_body);

    return ret;
}

inline int SendToUdpThread(work::SchedulerInterface* scheduler, const ConnGuid* conn_guid,
                           const MsgHead& msg_head, const google::protobuf::Message* protobuf_msg, int udp_thread_idx)
{
    char* msg_body = NULL;
    size_t msg_body_len = 0;

    if (SerializeProtobufMsg(&msg_body, msg_body_len, protobuf_msg) != 0)
    {
        return -1;
    }

    int ret = scheduler->SendToUdpThread(conn_guid, msg_head, msg_body, msg_body_len, udp_thread_idx);
    FreeProtobufMsgBuf(&msg_body);

    return ret;
}

#endif // APP_FRAME_INC_WORK_PROTOBUF_MSG_UTIL_H_
