#ifndef RAW_TCP_THREADS_SRC_BUFFER_EVENT_CONN_H_
#define RAW_TCP_THREADS_SRC_BUFFER_EVENT_CONN_H_

#include <event2/bufferevent.h>
#include "base_conn.h"
#include "mem_util.h"

#if defined(USE_BUFFEREVENT)
namespace tcp
{
namespace raw
{
class BufferEventConn : public BaseConn
{
    CREATE_FUNC(BufferEventConn)

private:
    static void EventCallback(struct bufferevent* buffer_event, short events, void* arg);
    static void ReadCallback(struct bufferevent* buffer_event, void* arg);

public:
    BufferEventConn();
    virtual ~BufferEventConn();

    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    int Send(const void* data, size_t len) override;

private:
    struct bufferevent* buffer_event_;
};
}
}
#endif

#endif // RAW_TCP_THREADS_SRC_BUFFER_EVENT_CONN_H_
