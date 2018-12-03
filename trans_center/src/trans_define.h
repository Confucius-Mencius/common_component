#ifndef TRANS_CENTER_SRC_TRANS_DEFINE_H_
#define TRANS_CENTER_SRC_TRANS_DEFINE_H_

#include <string.h>
#include "log_util.h"
#include "trans_center_interface.h"

namespace trans_center
{
class Trans
{
public:
    Trans() : trans_ctx_()
    {
    }

    ~Trans()
    {
    }

    static Trans* Create(const TransCtx* ctx)
    {
        Trans* trans = new Trans();
        if (NULL == trans)
        {
            const int err = errno;
            LOG_ERROR("failed to create trans, errno: " << err << ", err msg: " << strerror(err));
            return NULL;
        }

        trans->trans_ctx_.peer = ctx->peer;
        trans->trans_ctx_.timeout_sec = ctx->timeout_sec;
        trans->trans_ctx_.passback = ctx->passback;

        if (NULL == ctx->sink)
        {
            return trans;
        }

        trans->trans_ctx_.sink = ctx->sink;

        if (NULL == ctx->async_data || 0 == ctx->async_data_len)
        {
            return trans;
        }

        trans->trans_ctx_.async_data = new char[ctx->async_data_len + 1];
        if (NULL == trans->trans_ctx_.async_data)
        {
            const int err = errno;
            LOG_ERROR("failed to create async data buf, errno: " << err << ", err msg: " << strerror(err));

            delete trans;
            return NULL;
        }

        memcpy(trans->trans_ctx_.async_data, ctx->async_data, ctx->async_data_len);
        trans->trans_ctx_.async_data_len = ctx->async_data_len;

        return trans;
    }

    void Release()
    {
        if (trans_ctx_.async_data != NULL)
        {
            delete[] trans_ctx_.async_data;
            trans_ctx_.async_data = NULL;
            trans_ctx_.async_data_len = 0;
        }

        delete this;
    }

    const TransCtx* GetCtx() const
    {
        return &trans_ctx_;
    }

private:
    TransCtx trans_ctx_;
};
}

#endif // TRANS_CENTER_SRC_TRANS_DEFINE_H_
