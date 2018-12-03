#ifndef MSG_CODEC_CENTER_SRC_MSG_CODEC_H_
#define MSG_CODEC_CENTER_SRC_MSG_CODEC_H_

#include <event2/buffer.h>
#include "mem_util.h"
#include "msg_codec_interface.h"

namespace msg_codec_center
{
class MsgCodec : public MsgCodecInterface
{
    CREATE_FUNC(MsgCodec);

public:
    MsgCodec();
    virtual ~MsgCodec();

    ///////////////////////// MsgCodecInterface /////////////////////////
    bool IsWholeMsg(MsgId& err_msg_id, size_t& total_msg_len, struct evbuffer* ev_buf) override;
    bool IsWholeMsg(MsgId& err_msg_id, const char* data_buf, size_t data_len) override;
    int DecodeMsg(MsgId& err_msg_id, MsgHead* msg_head, char** msg_body, size_t& msg_body_len,
                  const char* total_msg_buf, size_t total_msg_len) override;
    int EncodeMsg(char** data_buf, size_t& data_len, const MsgHead& msg_head, const void* msg_body,
                  size_t msg_body_len) override;

public:
    void Release();
    int Initialize(const void* ctx);
    void Finalize();
    int Activate();
    void Freeze();

private:
    MsgCodecCtx msg_codec_ctx_;
};
}

#endif // MSG_CODEC_CENTER_SRC_MSG_CODEC_H_
