#ifndef TRANS_CENTER_SRC_TRANS_H_
#define TRANS_CENTER_SRC_TRANS_H_

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
        if (nullptr == trans)
        {
            const int err = errno;
            LOG_ERROR("failed to create trans, errno: " << err << ", err msg: " << strerror(err));
            return nullptr;
        }

        trans->trans_ctx_.peer = ctx->peer;
        trans->trans_ctx_.timeout_sec = ctx->timeout_sec;
        trans->trans_ctx_.passback = ctx->passback;

        if (nullptr == ctx->sink)
        {
            return trans;
        }

        trans->trans_ctx_.sink = ctx->sink;

        if (nullptr == ctx->data || ctx->len < 1)
        {
            // 没有异步数据需要暂存
            return trans;
        }

        if (0 == ctx->len)
        {
            // 记录对象指针
            trans->trans_ctx_.data = ctx->data;
            trans->trans_ctx_.len = 0;
            return trans;
        }

        trans->trans_ctx_.data = new char[ctx->len + 1];
        if (nullptr == trans->trans_ctx_.data)
        {
            const int err = errno;
            LOG_ERROR("failed to create async data buf, errno: " << err << ", err msg: " << strerror(err));

            delete trans;
            return nullptr;
        }

        memcpy(trans->trans_ctx_.data, ctx->data, ctx->len);
        trans->trans_ctx_.len = ctx->len;

        return trans;
    }

    void Release()
    {
        if (trans_ctx_.data != nullptr && trans_ctx_.len > 0)
        {
            delete[] trans_ctx_.data;
            trans_ctx_.data = nullptr;
            trans_ctx_.len = 0;
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

#endif // TRANS_CENTER_SRC_TRANS_H_
