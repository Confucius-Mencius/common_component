#ifndef WS_THREADS_SRC_CONN_H_
#define WS_THREADS_SRC_CONN_H_

#include <list>
#include <libwebsockets.h>
#include "base_conn.h"
#include "mem_util.h"

namespace ws
{
class Conn : public BaseConn
{
    CREATE_FUNC(Conn)

public:
    Conn();
    virtual ~Conn();

    void SetWsi(struct lws* wsi)
    {
        wsi_ = wsi;
    }

    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    int Send(const void* data, size_t len) override;

    int SendListData();

private:
    struct lws* wsi_; // 客户端连接句柄

    typedef std::list<std::string> SendList; // data to send
    SendList send_list_;
};
}

#endif // WS_THREADS_SRC_CONN_H_
