#include "http_conn.h"
#include <sys/utsname.h>
#include "log_util.h"
#include "str_util.h"

namespace http
{
Conn::Conn()
{
    conn_center_ctx_ = NULL;
    https_ = false;
    evhttp_conn_ = NULL;
}

Conn::~Conn()
{
}

void Conn::Release()
{
    delete this;
}

int Conn::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    conn_center_ctx_ = (ConnCenterCtx*) ctx;
    return 0;
}

void Conn::Finalize()
{
    // 不要释放conn_,libevent会自动释放，否则会导致double free
}

int Conn::Activate()
{
    return 0;
}

void Conn::Freeze()
{
}

int Conn::Send(struct evhttp_request* evhttp_req, int http_code, const KeyValMap* header_map,
               const char* content, size_t content_len)
{
    struct utsname utsname_buf;
    uname(&utsname_buf);

    struct evkeyvalq* output_headers = evhttp_request_get_output_headers(evhttp_req);
    char buf[256] = "";

    StrPrintf(buf, sizeof(buf), "Libevent Http Server/%d.%d (%s %s/%s)", evhttp_req->major, evhttp_req->minor,
              utsname_buf.release, utsname_buf.version, utsname_buf.sysname);
    evhttp_add_header(output_headers, "Server", buf);

    if (conn_center_ctx_->content_type.length() > 0)
    {
        evhttp_add_header(output_headers, "Content-Type", conn_center_ctx_->content_type.c_str());
    }

    if (conn_center_ctx_->no_cache)
    {
        evhttp_add_header(output_headers, "Cache-Control", "no-cache");
        evhttp_add_header(output_headers, "Pragma", "no-cache");
    }

    if (header_map != NULL)
    {
        for (KeyValMap::const_iterator it = header_map->begin(); it != header_map->end(); ++it)
        {
            memset(buf, 0, sizeof(buf));
            it->second.ToString(buf, sizeof(buf));
            evhttp_add_header(output_headers, it->first.c_str(), buf);
        }
    }

    if (content != NULL && content_len > 0)
    {
        struct evbuffer* evbuf = evbuffer_new();
        if (NULL == evbuf)
        {
            const int err = errno;
            LOG_ERROR("failed to create evbuf, errno: " << err << ", err msg: " << strerror(err));
            evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
            return -1;
        }

        if (evbuffer_add(evbuf, content, content_len) != 0)
        {
            const int err = errno;
            LOG_ERROR("failed to add data to evbuf, errno: " << err << ", err msg: " << strerror(err));
            evhttp_send_error(evhttp_req, HTTP_INTERNAL, NULL);
        }
        else
        {
            evhttp_send_reply(evhttp_req, http_code, NULL, evbuf);
        }

        evbuffer_free(evbuf);
    }
    else
    {
        evhttp_send_reply(evhttp_req, http_code, NULL, NULL);
    }

    return 0;
}
}
