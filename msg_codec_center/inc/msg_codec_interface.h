#ifndef MSG_CODEC_CENTER_INC_MSG_CODEC_INTERFACE_H_
#define MSG_CODEC_CENTER_INC_MSG_CODEC_INTERFACE_H_

#include "msg_define.h"

struct evbuffer;

struct MsgCodecCtx
{
    size_t max_msg_body_len;
    bool do_checksum;

    MsgCodecCtx()
    {
        max_msg_body_len = 0;
        do_checksum = false;
    }
};

class MsgCodecInterface
{
public:
    virtual ~MsgCodecInterface()
    {
    }

    /**
     * @brief 判断收到的是否是一个完整的消息
     * @param err_msg_id 如果有错误，返回一个表示错误的msg id
     * @param total_msg_len 如果是完整的消息，返回消息的长度，包括checksum, msg head和msg body几部分
     * @param ev_buf libevent接收缓冲区
     * @return
     */
    virtual bool IsWholeMsg(MsgId& err_msg_id, size_t& total_msg_len, struct evbuffer* ev_buf) = 0;

    virtual bool IsWholeMsg(MsgId& err_msg_id, const char* data_buf, size_t data_len) = 0;

    /**
     * @brief 解码消息
     * @param err_msg_id 如果有错误，返回一个表示错误的msg id
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @param total_msg_buf 消息缓冲区，包括checksum, msg head和msg body几部分
     * @param total_msg_len 消息长度，包括checksum, msg head和msg body几部分
     * @return =0表示成功，否则失败
     */
    virtual int DecodeMsg(MsgId& err_msg_id, MsgHead* msg_head, char** msg_body, size_t& msg_body_len,
                          const char* total_msg_buf, size_t total_msg_len) = 0;

    /**
     * @brief 编码消息
     * @param data_buf 存放编码结果的缓冲区，包括total msg len, checksum, msg head和msg body几部分
     * @param data_len 编码结果的长度，包括total msg len, checksum, msg head和msg body几部分
     * @param msg_head
     * @param msg_body
     * @param msg_body_len
     * @return =0表示成功，否则失败
     */
    virtual int EncodeMsg(char** data_buf, size_t& data_len, const MsgHead& msg_head, const void* msg_body,
                          size_t msg_body_len) = 0;
};

#endif // MSG_CODEC_CENTER_INC_MSG_CODEC_INTERFACE_H_
