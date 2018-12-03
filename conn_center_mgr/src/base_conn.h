#ifndef CONN_CENTER_MGR_SRC_BASE_CONN_H_
#define CONN_CENTER_MGR_SRC_BASE_CONN_H_

#include <time.h>
#include "common_define.h"

namespace conn_center_mgr
{
class BaseConn
{
public:
    BaseConn();
    virtual ~BaseConn();

    virtual void Release() = 0;
    virtual int Initialize(const void* ctx) = 0;
    virtual void Finalize() = 0;
    virtual int Activate() = 0;
    virtual void Freeze() = 0;

public:
    void SetCreatedTime(time_t t)
    {
        created_time_ = t;
    }

    void SetConnGuid(int io_thread_type, int io_thread_idx, ConnId conn_id)
    {
        conn_guid_.io_thread_type = io_thread_type;
        conn_guid_.io_thread_idx = io_thread_idx;
        conn_guid_.conn_id = conn_id;
    }

    ConnId GetConnId() const
    {
        return conn_guid_.conn_id;
    }

protected:
    time_t created_time_;
    ConnGuid conn_guid_;
    std::string client_ip_;
    unsigned short client_port_;
};
}

#endif // CONN_CENTER_MGR_SRC_BASE_CONN_H_
