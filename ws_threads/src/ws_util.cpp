#include "ws_util.h"
#include "log_util.h"

namespace ws
{
void DumpRequestHeaders(struct lws* wsi)
{
    int n = 0, len;
    char buf[256];
    const unsigned char* c;

    do
    {
        c = lws_token_to_string(lws_token_indexes(n));
        if (!c)
        {
            n++;
            continue;
        }

        len = lws_hdr_total_length(wsi, (lws_token_indexes) n);
        if (!len || len > int(sizeof(buf) - 1))
        {
            n++;
            continue;
        }

        lws_hdr_copy(wsi, buf, sizeof buf, (lws_token_indexes) n);
        buf[sizeof(buf) - 1] = '\0';

        LOG_DEBUG((char*) c << " = " << buf);
        n++;
    } while (c);
}
}
