#ifndef APP_FRAME_INC_CONN_DEFINE_H_
#define APP_FRAME_INC_CONN_DEFINE_H_

#include <ostream>
#include "seq_num.h"

// connection id
typedef int32_t ConnID; /**< ConnId类型 */
typedef I32SeqNum ConnIDSeq; /**< ConnIdSeq类型 */

#ifndef INVALID_CONN_ID
/**
 * @brief invalid conn id
 */
#define INVALID_CONN_ID INVALID_SEQ_NUM
#endif // INVALID_CONN_ID

enum IOThreadType
{
    IO_THREAD_TYPE_MIN = 0,
    IO_THREAD_TYPE_TCP = IO_THREAD_TYPE_MIN, // tcp服务
    IO_THREAD_TYPE_WS,  // websocket服务
    IO_THREAD_TYPE_UDP, // udp服务
    IO_THREAD_TYPE_MAX,
};

struct ConnGUID
{
    IOThreadType io_thread_type;
    int io_thread_idx;
    ConnID conn_id;

    // 动态创建
    static ConnGUID* Create(const ConnGUID* instance)
    {
        ConnGUID* obj = new ConnGUID();
        if (NULL == obj)
        {
            return NULL;
        }

        obj->io_thread_type = instance->io_thread_type;
        obj->io_thread_idx = instance->io_thread_idx;
        obj->conn_id = instance->conn_id;

        return obj;
    }

    // 释放
    void Release()
    {
        delete this;
    }

    ConnGUID()
    {
        io_thread_type = IO_THREAD_TYPE_MAX;
        io_thread_idx = -1;
        conn_id = INVALID_CONN_ID;
    }

    ConnGUID(IOThreadType io_thread_type, int io_thread_idx, ConnID conn_id)
    {
        this->io_thread_type = io_thread_type;
        this->io_thread_idx = io_thread_idx;
        this->conn_id = conn_id;
    }

    bool operator<(const ConnGUID& rhs) const
    {
        if (io_thread_type != rhs.io_thread_type)
        {
            return io_thread_type < rhs.io_thread_type;
        }
        else if (io_thread_idx != rhs.io_thread_idx)
        {
            return io_thread_idx < rhs.io_thread_idx;
        }
        else
        {
            return conn_id < rhs.conn_id;
        }
    }

    friend std::ostream& operator<<(std::ostream& os, const ConnGUID& instance)
    {
        os << "[conn guid]io thread type: " << instance.io_thread_type
           << ", io thread idx: " << instance.io_thread_idx
           << ", conn id: " << instance.conn_id;
        return os;
    }
};

#endif // APP_FRAME_INC_CONN_DEFINE_H_
