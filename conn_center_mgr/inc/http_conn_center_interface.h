#ifndef CONN_CENTER_MGR_INC_HTTP_CONN_CENTER_INTERFACE_H_
#define CONN_CENTER_MGR_INC_HTTP_CONN_CENTER_INTERFACE_H_

#include <string>
#include "common_define.h"

class TimerAxisInterface;
struct evhttp_request;
struct evhttp_connection;

namespace http
{
struct ConnCenterCtx
{
    TimerAxisInterface* timer_axis;
    std::string content_type;
    bool no_cache;

    ConnCenterCtx() : content_type("")
    {
        timer_axis = NULL;
        no_cache = false;
    }
};

class ConnInterface
{
public:
    virtual ~ConnInterface()
    {
    }

    virtual const ConnGuid& GetConnGuid() const = 0;
    virtual void SetClientIpPort(const char* ip, unsigned short port) = 0;
    virtual const char* GetClientIp() const = 0;
    virtual unsigned short GetClientPort() const = 0;
    virtual int Send(struct evhttp_request* evhttp_req, int http_code, const KeyValMap* header_map,
                     const char* content, size_t content_len) = 0;
};

class ConnCenterInterface
{
public:
    virtual ~ConnCenterInterface()
    {
    }

    virtual ConnInterface* CreateConn(int io_thread_idx, struct evhttp_connection* evhttp_conn, bool https) = 0;

    /**
     * @brief
     * @param evhttp_conn
     * @attention 其中会释放conn对象
     */
    virtual void RemoveConn(struct evhttp_connection* evhttp_conn) = 0;

    virtual ConnInterface* GetConn(struct evhttp_connection* evhttp_conn) const = 0;
    virtual ConnInterface* GetConnByConnId(ConnId conn_id) const = 0;
};
}

#endif // CONN_CENTER_MGR_INC_HTTP_CONN_CENTER_INTERFACE_H_
