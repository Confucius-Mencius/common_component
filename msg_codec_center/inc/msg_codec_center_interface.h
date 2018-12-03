#ifndef MSG_CODEC_CENTER_INC_MSG_CODEC_CENTER_INTERFACE_H_
#define MSG_CODEC_CENTER_INC_MSG_CODEC_CENTER_INTERFACE_H_

#include "module_interface.h"
#include "msg_codec_interface.h"

class MsgCodecCenterInterface : public ModuleInterface
{
public:
    virtual ~MsgCodecCenterInterface()
    {
    }

    virtual MsgCodecInterface* CreateMsgCodec(const MsgCodecCtx* msg_codec_ctx) = 0;
};

#endif // MSG_CODEC_CENTER_INC_MSG_CODEC_CENTER_INTERFACE_H_
