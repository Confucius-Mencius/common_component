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

    void SetWSI(struct lws* wsi)
    {
        wsi_ = wsi;
    }

    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    std::string& AppendData(const void* data, size_t len)
    {
        return data_.append((const char*) data, len);
    }

    void EraseData()
    {
        data_.clear();
    }

    int Send(const void* data, size_t len) override;
    int OnWrite();

private:
    struct lws* wsi_; // 客户端连接句柄
    std::string data_; // received data
    typedef std::list<std::string> DataList; // data to send
    DataList data_list_;
};
}

#endif // WS_THREADS_SRC_CONN_H_
