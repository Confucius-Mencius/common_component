#include "conn.h"
#include "log_util.h"
#include "thread_sink.h"

namespace ws
{
Conn::Conn() : send_list_()
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
    send_list_.clear();
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

    char* buf = (char*) malloc(LWS_PRE + len + 1);
    if (NULL == buf)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc memory, err: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    memcpy(buf + len, data, len);
    buf[LWS_PRE + len] = '\0';

    send_list_.push_back(std::string(buf, LWS_PRE + len));
    free(buf);

    lws_callback_on_writable(wsi_);

    return 0;
}

int Conn::SendBinary()
{
    if (NULL == wsi_)
    {
        return -1;
    }

    for (SendList::const_iterator it = send_list_.begin(); it != send_list_.end(); ++it)
    {
        const int m = lws_write(wsi_, (unsigned char*) it->data() + LWS_PRE, it->size() - LWS_PRE, LWS_WRITE_BINARY);
        if (m < (int) (it->size() - LWS_PRE))
        {
            const int err = errno;
            LOG_ERROR("lws_write failed, err: " << err << ", err msg: " << strerror(err));
            return -1;
        }
    }

    return 0;
}
}
