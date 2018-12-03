#ifndef MSG_CODEC_CENTER_SRC_MSG_CODEC_CENTER_H_
#define MSG_CODEC_CENTER_SRC_MSG_CODEC_CENTER_H_

#include <set>
#include "msg_codec.h"
#include "msg_codec_center_interface.h"

namespace msg_codec_center
{
class MsgCodecCenter : public MsgCodecCenterInterface
{
public:
    MsgCodecCenter();
    virtual ~MsgCodecCenter();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// MsgCodecCenterInterface /////////////////////////
    MsgCodecInterface* CreateMsgCodec(const MsgCodecCtx* msg_codec_ctx) override;

private:
    typedef std::set<MsgCodec*> MsgCodecSet;
    MsgCodecSet msg_codec_set_;
};
}

#endif // MSG_CODEC_CENTER_SRC_MSG_CODEC_CENTER_H_
