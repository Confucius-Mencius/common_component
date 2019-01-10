#ifndef TCP_THREADS_SRC_BUFFER_EVENT_CONN_H_
#define TCP_THREADS_SRC_BUFFER_EVENT_CONN_H_

#include <event2/bufferevent.h>
#include "base_conn.h"
#include "mem_util.h"

namespace tcp
{
class BufferEventConn : public BaseConn
{
    CREATE_FUNC(BufferEventConn)

public:
    BufferEventConn();
    virtual ~BufferEventConn();

    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    int Send(const void* data, size_t len) override;

    void SetBufferEvent(struct bufferevent* buffer_event)
    {
        buffer_event_ = buffer_event;
    }

private:
    struct bufferevent* buffer_event_;
};
}

#endif // TCP_THREADS_SRC_BUFFER_EVENT_CONN_H_
