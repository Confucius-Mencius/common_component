#ifndef PROTO_MSG_CODEC_SRC_MSG_CODEC_H_
#define PROTO_MSG_CODEC_SRC_MSG_CODEC_H_

#include <event2/buffer.h>
#include "mem_util.h"
#include "proto_msg_codec_interface.h"

namespace proto
{
class MsgCodec : public MsgCodecInterface
{
    CREATE_FUNC(MsgCodec)

public:
    MsgCodec();
    virtual ~MsgCodec();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// MsgCodecInterface /////////////////////////
//    bool IsWholeMsg(MsgID& err_msg_id, size_t& total_msg_len, struct evbuffer* ev_buf) override;
    bool IsWholeMsg(MsgID& err_msg_id, size_t& total_msg_len, const char* data, size_t len) override;
    int DecodeMsg(MsgID& err_msg_id, MsgHead* msg_head, char** msg_body, size_t& msg_body_len,
                  const char* total_msg_buf, size_t total_msg_len) override;
    int EncodeMsg(char** data_buf, size_t& data_len, const MsgHead& msg_head, const void* msg_body,
                  size_t msg_body_len) override;

private:
    MsgCodecCtx msg_codec_ctx_;
};
}

#endif // PROTO_MSG_CODEC_SRC_MSG_CODEC_H_
