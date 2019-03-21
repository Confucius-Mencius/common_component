#include "ws_util.h"
#include "log_util.h"

namespace ws
{
void DumpAllToken(struct lws* wsi)
{
    for (int i = 0; i < WSI_TOKEN_COUNT; ++i)
    {
        const unsigned char* token = lws_token_to_string((lws_token_indexes) i);
        if (NULL == token)
        {
            continue;
        }

        const int len = lws_hdr_total_length(wsi, (lws_token_indexes) i);
        if (len <= 0)
        {
            continue;
        }

        std::unique_ptr<char[]> buf(new char[len + 1]);
        if (nullptr == buf)
        {
            LOG_ERROR("failed to alloc memory");
            continue;
        }

        lws_hdr_copy(wsi, buf.get(), len + 1, (lws_token_indexes) i);
        buf[len] = '\0';

        LOG_DEBUG((char*) token << " = " << buf.get());
    }
}
}
