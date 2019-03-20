#include "http_conn.h"
#include "log_util.h"
#include "thread_sink.h"

namespace ws
{
namespace http
{
Conn::Conn() : data_(), data_list_()
{
    wsi_ = NULL;
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
    (void) ctx;
    return 0;
}

void Conn::Finalize()
{
    data_list_.clear();
}

int Conn::Activate()
{
    return 0;
}

void Conn::Freeze()
{
}

int Conn::Send(const void* data, size_t len)
{
    if (NULL == wsi_)
    {
        return -1;
    }

    std::string d;
    d.append(LWS_PRE, '\0');
    d.append((const char*) data, len);

    data_list_.emplace_back(d);
    LOG_TRACE("push sending data into list, len: " << len);

    lws_callback_on_writable(wsi_); // 个调用会触发LWS_CALLBACK_SERVER_WRITEABLE，参数wsi_是这个客户端连接的wsi
    return 0;
}

int Conn::OnWrite()
{
    if (NULL == wsi_)
    {
        return -1;
    }

    for (DataList::iterator it = data_list_.begin(); it != data_list_.end();)
    {
        const int n = lws_write(wsi_, (unsigned char*) it->data() + LWS_PRE, it->size() - LWS_PRE, LWS_WRITE_BINARY);
        if (n == (int) (it->size() - LWS_PRE))
        {
            LOG_TRACE("send ok, n: " << n << ", " << conn_guid_);
            it = data_list_.erase(it);
        }
        else
        {
            const int err = errno;
            LOG_ERROR("lws_write failed, err: " << err << ", err msg: " << strerror(err));
            ++it;
        }
    }

    return 0;
}
}
}
