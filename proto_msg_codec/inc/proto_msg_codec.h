#ifndef PROTO_MSG_CODEC_INC_PROTO_MSG_CODEC_H_
#define PROTO_MSG_CODEC_INC_PROTO_MSG_CODEC_H_

#include <arpa/inet.h>
#include <string.h>
#include "fast_crc32.h"
#include "log_util.h"
#include "proto_msg.h"

namespace proto
{
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

class MsgCodec
{
public:
    MsgCodec() {}
    ~MsgCodec() {}

    void SetCtx(const MsgCodecCtx* msg_codec_ctx) { msg_codec_ctx_ = *msg_codec_ctx; }

    bool IsWholeMsg(MsgID& err_msg_id, size_t& total_msg_len, const char* data, size_t len)
    {
        static const size_t max_total_msg_len = MIN_TOTAL_MSG_LEN + msg_codec_ctx_.max_msg_body_len;

        total_msg_len = ntohl(*(int32_t*) data);
        if (total_msg_len < MIN_TOTAL_MSG_LEN || total_msg_len > max_total_msg_len)
        {
            LOG_ERROR("invalid msg len: " << total_msg_len << ", throw away all bytes in the buf");
            err_msg_id = MSG_ID_INVALID_MSG_LEN;
            return false;
        }

        if (len < (TOTAL_MSG_LEN_FIELD_LEN + total_msg_len))
        {
            err_msg_id = MSG_ID_NOT_A_WHOLE_MSG;
            return false;
        }

        LOG_DEBUG("recv a whole msg, msg len: " << total_msg_len);
        return true;
    }

    int DecodeMsg(MsgID& err_msg_id, MsgHead* msg_head, char** msg_body, size_t& msg_body_len,
                  const char* total_msg_buf, size_t total_msg_len)
    {
        uint32_t checksum_self = 0;

        if (msg_codec_ctx_.do_checksum)
        {
            const uint32_t checksum_peer = ntohl(*((uint32_t*) total_msg_buf));
            checksum_self = FastCRC32(total_msg_buf + CHECKSUM_FIELD_LEN, total_msg_len - CHECKSUM_FIELD_LEN);
            if (checksum_peer != checksum_self)
            {
                LOG_ERROR("checksum not match, self checksum: " << checksum_self << ", peer checksum: " << checksum_peer);
                err_msg_id = MSG_ID_CHECKSUM_MISMATCH;
                return -1;
            }
        }

        msg_head->passback = ntohl(*(int32_t*) (total_msg_buf + CHECKSUM_FIELD_LEN));
        msg_head->msg_id = ntohl(*(int32_t*) (total_msg_buf + CHECKSUM_FIELD_LEN + PASSBACK_FIELD_LEN));

        *msg_body = NULL;
        msg_body_len = 0;

        if (total_msg_len > MIN_TOTAL_MSG_LEN)
        {
            *msg_body = ((char*) total_msg_buf) + MIN_TOTAL_MSG_LEN;
            msg_body_len = total_msg_len - MIN_TOTAL_MSG_LEN;
        }

        LOG_DEBUG("decode msg ok, checksum: " << checksum_self << ", " << *msg_head << ", msg body len: " << msg_body_len);
        return 0;
    }

    int EncodeMsg(char** data, size_t& len, const MsgHead& msg_head, const void* msg_body,
                  size_t msg_body_len)
    {
        *((int32_t*) (*data)) = htonl(MIN_TOTAL_MSG_LEN + msg_body_len);
        *((int32_t*) (*data + TOTAL_MSG_LEN_FIELD_LEN + CHECKSUM_FIELD_LEN)) = htonl(msg_head.passback);
        *((int32_t*) (*data + TOTAL_MSG_LEN_FIELD_LEN + CHECKSUM_FIELD_LEN + PASSBACK_FIELD_LEN)) = htonl(msg_head.msg_id);

        if (msg_body != NULL)
        {
            memcpy(*data + TOTAL_MSG_LEN_FIELD_LEN + MIN_TOTAL_MSG_LEN, msg_body, msg_body_len);
        }

        uint32_t checksum = 0;

        if (msg_codec_ctx_.do_checksum)
        {
            checksum = FastCRC32(*data + TOTAL_MSG_LEN_FIELD_LEN + CHECKSUM_FIELD_LEN,
                                 MIN_TOTAL_MSG_LEN - CHECKSUM_FIELD_LEN + msg_body_len);
        }

        *((uint32_t*) (*data + TOTAL_MSG_LEN_FIELD_LEN)) = htonl(checksum);
        len = TOTAL_MSG_LEN_FIELD_LEN + MIN_TOTAL_MSG_LEN + msg_body_len;

        LOG_DEBUG("encode msg ok, checksum: " << checksum << ", " << msg_head << ", msg body len: " << msg_body_len);
        return 0;
    }

private:
    MsgCodecCtx msg_codec_ctx_;
};
}

#endif // PROTO_MSG_CODEC_INC_PROTO_MSG_CODEC_H_
