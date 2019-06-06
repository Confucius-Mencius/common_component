#ifndef PROTO_MSG_CODEC_INC_PROTO_MSG_H_
#define PROTO_MSG_CODEC_INC_PROTO_MSG_H_

#include <ostream>
#include "std_int.h"

namespace proto
{
typedef int32_t Passback; /**< Passback类型 */
typedef int32_t MsgID; /**< MsgID类型 */

#define NFY_PASSBACK ((int32_t) 0x80000000) // 标识nfy消息
#define NAN_PASSBACK ((int32_t) 0) // 无意义，不用关注的passback

// 特殊的msg id
#define MSG_ID_OK                   0
#define MSG_ID_NOT_A_WHOLE_MSG      -101
//#define MSG_ID_COPY_OUT_FAILED      -102 // 从libevent buffer中拷贝数据失败
//#define MSG_ID_REMOVE_OUT_FAILED    -103 // 从libevent buffer中移出数据失败
#define MSG_ID_INVALID_MSG_LEN      -104
#define MSG_ID_CHECKSUM_MISMATCH    -105
#define MSG_ID_NONE_HANDLER_FOUND   -106
//#define MSG_ID_IO_TO_WORK_TQ_FULL   -107
//#define MSG_ID_ALLOC_MEM_FAILED     -108
//#define MSG_ID_INIT_TASK_FAILED     -109
//#define MSG_ID_WRITE_PIPE_FAILED    -110
#define MSG_ID_SCHEDULE_FAILED      -111
#define MSG_ID_HEARTBEAT_REQ        -200
#define MSG_ID_HEARTBEAT_RSP        -201

// 消息结构：| total msg len(int32_t) | checksum(uint32_t) | passback(int32_t) | msg id(int32_t) | msg body() |
#define TOTAL_MSG_LEN_FIELD_LEN sizeof(int32_t)
#define CHECKSUM_FIELD_LEN sizeof(uint32_t)
#define PASSBACK_FIELD_LEN sizeof(int32_t)
#define MSG_ID_FIELD_LEN sizeof(int32_t)

#define MSG_HEAD_LEN (PASSBACK_FIELD_LEN + MSG_ID_FIELD_LEN)
#define MIN_TOTAL_MSG_LEN (CHECKSUM_FIELD_LEN + MSG_HEAD_LEN)
#define MIN_DATA_LEN (TOTAL_MSG_LEN_FIELD_LEN + MIN_TOTAL_MSG_LEN)

#define MSG_HEAD_INITIALIZER() passback = 0; msg_id = 0

struct MsgHead
{
    Passback passback;
    MsgID msg_id;

    MsgHead()
    {
        MSG_HEAD_INITIALIZER();
    }

    MsgHead(Passback passback, MsgID msg_id)
    {
        this->passback = passback;
        this->msg_id = msg_id;
    }

    ~MsgHead()
    {
    }

    void Reset()
    {
        MSG_HEAD_INITIALIZER();
    }

    MsgHead& operator=(const MsgHead& rhs)
    {
        if (&rhs != this)
        {
            passback = rhs.passback;
            msg_id = rhs.msg_id;
        }

        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, const MsgHead& instance)
    {
        os << "msg head => {passback: " << instance.passback << ", msg id: " << instance.msg_id << "}";
        return os;
    }
};
}

#endif // PROTO_MSG_CODEC_INC_PROTO_MSG_H_
