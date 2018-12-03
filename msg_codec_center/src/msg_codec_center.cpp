#include "msg_codec_center.h"
#include <string.h>
#include "container_util.h"
#include "log_util.h"
#include "version.h"

namespace msg_codec_center
{
MsgCodecCenter::MsgCodecCenter() : msg_codec_set_()
{
}

MsgCodecCenter::~MsgCodecCenter()
{
}

const char* MsgCodecCenter::GetVersion() const
{
    return MSG_CODEC_CENTER_MSG_CODEC_CENTER_VERSION;
}

const char* MsgCodecCenter::GetLastErrMsg() const
{
    return nullptr;
}

void MsgCodecCenter::Release()
{
    RELEASE_CONTAINER(msg_codec_set_);
    delete this;
}

int MsgCodecCenter::Initialize(const void* ctx)
{
    return 0;
}

void MsgCodecCenter::Finalize()
{
    FINALIZE_CONTAINER(msg_codec_set_);
}

int MsgCodecCenter::Activate()
{
    return 0;
}

void MsgCodecCenter::Freeze()
{
    FREEZE_CONTAINER(msg_codec_set_);
}

MsgCodecInterface* MsgCodecCenter::CreateMsgCodec(const MsgCodecCtx* msg_codec_ctx)
{
    MsgCodec* msg_codec = MsgCodec::Create();
    if (NULL == msg_codec)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc memory, errno: " << err << ", err msg: " << strerror(err));
        return NULL;
    }

    int ret = -1;

    do
    {
        if (msg_codec->Initialize(msg_codec_ctx) != 0)
        {
            break;
        }

        if (msg_codec->Activate() != 0)
        {
            break;
        }

        if (!msg_codec_set_.insert(msg_codec).second)
        {
            const int err = errno;
            LOG_ERROR("failed to insert to set, msg codec: " << msg_codec
                      << ", errno: " << err << ", err msg: " << strerror(err));
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        msg_codec_set_.erase(msg_codec);
        SAFE_DESTROY(msg_codec);
        return NULL;
    }

    return msg_codec;
}
}
