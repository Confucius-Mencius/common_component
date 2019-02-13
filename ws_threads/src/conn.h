#ifndef WS_THREADS_SRC_CONN_H_
#define WS_THREADS_SRC_CONN_H_

#include <list>
#include <event2/event.h>
#include "base_conn.h"
#include "mem_util.h"

namespace ws
{
class Conn : public BaseConn
{
    CREATE_FUNC(Conn)

private:
    static void ReadCallback(evutil_socket_t fd, short events, void* arg);
    static void WriteCallback(evutil_socket_t fd, short events, void* arg);

public:
    Conn();
    virtual ~Conn();

    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    int Send(const void* data, size_t len) override;

private:
    struct event* read_event_;

    typedef std::list<std::string> SendList; // data to send
    SendList send_list_;

    struct event* write_event_;
};
}

#endif // WS_THREADS_SRC_CONN_H_
