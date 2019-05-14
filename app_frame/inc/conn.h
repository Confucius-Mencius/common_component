#ifndef APP_FRAME_INC_CONN_H_
#define APP_FRAME_INC_CONN_H_

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

enum IOType
{
    IO_TYPE_MIN = 0,
    IO_TYPE_TCP = IO_TYPE_MIN,
    IO_TYPE_PROTO_TCP,
    IO_TYPE_WEB_TCP,
    IO_TYPE_UDP,
    IO_TYPE_MAX,
};

struct ConnGUID
{
    IOType io_type;
    int io_thread_idx;
    ConnID conn_id;

    // 动态创建
    static ConnGUID* Create(const ConnGUID* instance)
    {
        ConnGUID* obj = new ConnGUID();
        if (nullptr == obj)
        {
            return nullptr;
        }

        obj->io_type = instance->io_type;
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
        io_type = IO_TYPE_MAX;
        io_thread_idx = -1;
        conn_id = INVALID_CONN_ID;
    }

    ConnGUID(IOType io_type, int io_thread_idx, ConnID conn_id)
    {
        this->io_type = io_type;
        this->io_thread_idx = io_thread_idx;
        this->conn_id = conn_id;
    }

    bool operator<(const ConnGUID& rhs) const
    {
        if (io_type != rhs.io_type)
        {
            return io_type < rhs.io_type;
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
        os << "conn guid => { io type: " << instance.io_type
           << ", io thread idx: " << instance.io_thread_idx
           << ", conn id: " << instance.conn_id << " }";
        return os;
    }
};

class ConnInterface
{
public:
    virtual ~ConnInterface()
    {
    }

    virtual const ConnGUID* GetConnGUID() const = 0;
    virtual const char* GetClientIP() const = 0;
    virtual unsigned short GetClientPort() const = 0;
    // virtual int GetSockFD() const = 0;

    // 缓存收到的数据
    virtual std::string& AppendData(const char* data, size_t len) = 0;
    virtual void ClearData() = 0;
    virtual std::string& GetData() = 0;
};


class ConnCenterInterface
{
public:
    virtual ~ConnCenterInterface()
    {
    }

    virtual ConnInterface* GetConnBySockFD(int sock_fd) const = 0;
    virtual ConnInterface* GetConnByID(ConnID conn_id) const = 0;
};

#endif // APP_FRAME_INC_CONN_H_
