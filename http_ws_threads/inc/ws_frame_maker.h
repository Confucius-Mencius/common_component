#ifndef HTTP_WS_THREADS_INC_WS_FRAME_MAKER_H_
#define HTTP_WS_THREADS_INC_WS_FRAME_MAKER_H_

#include <string>
#include "std_int.h"

namespace tcp
{
namespace ws
{
class FrameMaker
{
public:
    enum
    {
        CONTINUATION = 0x00,
        TEXT = 0x01,
        BINARY = 0x02,
        CONNECTION_CLOSE = 0x08,
        PING = 0x09,
        PONG = 0xa
    };

    FrameMaker()
    {
    }

    FrameMaker& ResetMaskingKey()
    {
        frame_header_.mask = 0x00;
        return *this;
    }

    FrameMaker& SetMaskingKey(uint32_t masking_key)
    {
        frame_header_.mask = 0x80;
        frame_header_.masking_key = masking_key;
        return *this;
    }

    FrameMaker& SetFin(bool val)
    {
        frame_header_.fin = val ? 0x80 : 0;
        return *this;
    }

    FrameMaker& SetFrameType(int type)
    {
        frame_header_.opcode = type;
        return *this;
    }

    std::string MakeFrame(const char* data, size_t len)
    {
        std::string frame;
        int externded_payload_len = (len <= 125 ? 0 : (len <= 65535 ? 2 : 8));
        int mask_key_len = ((len && frame_header_.mask) ? 4 : 0);

        size_t frame_size =
            2 +
            externded_payload_len
            +
            mask_key_len
            +
            len;

        frame.resize(frame_size);
        uint8_t* ptr = (uint8_t*)frame.data();
        uint64_t offset = 0;

        ptr[0] |= frame_header_.fin;
        ptr[0] |= frame_header_.opcode;

        if (len)
        {
            ptr[1] |= frame_header_.mask;
        }

        offset += 1;
        if (len <= 125)
        {
            ptr[offset++] |= (unsigned char)len;
        }
        else if (len <= 65535)
        {
            ptr[offset++] |= 126;
            ptr[offset++] = (unsigned char)(len >> 8) & 0xFF;
            ptr[offset++] = len & 0xFF;
        }
        else
        {
            ptr[offset++] |= 127;
            ptr[offset++] = (unsigned char)(((uint64_t)len >> 56) & 0xff);
            ptr[offset++] = (unsigned char)(((uint64_t)len >> 48) & 0xff);
            ptr[offset++] = (unsigned char)(((uint64_t)len >> 40) & 0xff);
            ptr[offset++] = (unsigned char)(((uint64_t)len >> 32) & 0xff);
            ptr[offset++] = (unsigned char)(((uint64_t)len >> 24) & 0xff);
            ptr[offset++] = (unsigned char)(((uint64_t)len >> 16) & 0xff);
            ptr[offset++] = (unsigned char)(((uint64_t)len >> 8) & 0xff);
            ptr[offset++] = (unsigned char)((uint64_t)len & 0xff);
        }

        if (!len)
        {
            return frame;
        }

        if (frame_header_.mask)
        {
            int mask_key = frame_header_.masking_key;
            ptr[offset++] = (unsigned char)((mask_key >> 24) & 0xff);
            ptr[offset++] = (unsigned char)((mask_key >> 16) & 0xff);
            ptr[offset++] = (unsigned char)((mask_key >> 8) & 0xff);
            ptr[offset++] = (unsigned char)((mask_key) & 0xff);

            unsigned char* mask = ptr + offset - 4;
            for (uint32_t i = 0; i < len; i++)
            {
                ptr[offset++] = data[i] ^ mask[i % 4];
            }
        }
        else
        {
            memcpy((void*)(ptr + offset), data, (size_t)len);
            offset += len;
        }

        assert(offset == frame_size);
        return frame;
    }

private:
    struct FrameHeader
    {
        uint8_t fin = 0;
        uint8_t rsv1 = 0;
        uint8_t rsv2 = 0;
        uint8_t rsv3 = 0;
        int opcode = CONTINUATION;
        uint8_t mask = 0;
        uint8_t payload_len = 0;
        uint16_t ext_payload_len_16 = 0; //extended payload length 16
        uint64_t ext_payload_len_64 = 0; //extended payload length 64
        uint64_t payload_realy_len = 0;
        uint32_t masking_key = 0;
    };

    FrameHeader frame_header_;
};
}
}

#endif // HTTP_WS_THREADS_INC_WS_FRAME_MAKER_H_
