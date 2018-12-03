#ifndef CONN_CENTER_MGR_SRC_HTTP_CONN_H_
#define CONN_CENTER_MGR_SRC_HTTP_CONN_H_

// 代理外部发来的http连接

#include <evhttp.h>
#include "base_conn.h"
#include "http_conn_center_interface.h"
#include "mem_util.h"

namespace http
{
class Conn : public conn_center_mgr::BaseConn, public ConnInterface
{
    CREATE_FUNC(Conn);

public:
    Conn();
    virtual ~Conn();

    ///////////////////////// BaseConn /////////////////////////
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ConnInterface /////////////////////////
    const ConnGuid& GetConnGuid() const override
    {
        return conn_guid_;
    }

    void SetClientIpPort(const char* ip, unsigned short port) override
    {
        client_ip_ = ip;
        client_port_ = port;
    }

    const char* GetClientIp() const override
    {
        return client_ip_.c_str();
    }

    unsigned short GetClientPort() const override
    {
        return client_port_;
    }

    int Send(struct evhttp_request* evhttp_req, int http_code, const KeyValMap* header_map,
             const char* content, size_t content_len) override;

public:
    void SetEvhttpConn(struct evhttp_connection* evhttp_conn, bool https)
    {
        evhttp_conn_ = evhttp_conn;
        https_ = https;
    }

    struct evhttp_connection* GetEvhttpConn() const
    {
        return evhttp_conn_;
    }

    // client ip/port要从req中再次获取并设置进来，有可能多次跳转带在http头部字段（例如x-forward-for）中
    void SetClientIp(const char* ip)
    {
        if (NULL == ip)
        {
            return;
        }

        client_ip_ = ip;
    }

    void SetClientPort(unsigned short client_port)
    {
        client_port_ = client_port;
    }

private:
    const ConnCenterCtx* conn_center_ctx_;
    bool https_;
    struct evhttp_connection* evhttp_conn_;
};
}

#endif // CONN_CENTER_MGR_SRC_HTTP_CONN_H_
