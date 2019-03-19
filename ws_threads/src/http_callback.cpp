#include "http_callback.h"
#include "log_util.h"
#include "ws_util.h"

namespace ws
{

int HTTPCallback(lws* wsi, lws_callback_reasons reason, void* user, void* in, size_t len)
{
    LOG_TRACE("wsi: " << wsi << ", reason: " << reason << ", user: " << user << ", in: " << in << ", len: " << len);

    uint8_t buf[LWS_PRE + LWS_RECOMMENDED_MIN_HEADER_SPACE];
    uint8_t* start = &buf[LWS_PRE];
    uint8_t* p = start;
    uint8_t* end = &buf[sizeof(buf) - 1];

    switch (reason)
    {
        case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
        {
            LOG_TRACE("LWS_CALLBACK_FILTER_HTTP_CONNECTION");
            DumpRequestHeaders(wsi);
        }
        break;

        case LWS_CALLBACK_HTTP:
        {
            /*
             * `in` contains the url part after our mountpoint /sse, if any
             * you can use this to determine what data to return and store
             * that in the pss
             */
            LOG_TRACE("LWS_CALLBACK_HTTP");
            std::string url((const char*) in, len);
            LOG_DEBUG("url: " << url);

            if (lws_add_http_common_headers(wsi, HTTP_STATUS_OK,
                                            "text/html",
                                            LWS_ILLEGAL_HTTP_CONTENT_LEN, /* no content len */
                                            &p, end))
            {
                return 1;
            }
            if (lws_finalize_write_http_header(wsi, start, &p, end))
            {
                return 1;
            }

//            pss->established = time(NULL);

            /* SSE requires a response with this content-type */

            if (lws_add_http_common_headers(wsi, HTTP_STATUS_OK,
                                            "text/event-stream",
                                            LWS_ILLEGAL_HTTP_CONTENT_LEN,
                                            &p, end))
            {
                return 1; // 关闭连接
            }

            if (lws_finalize_write_http_header(wsi, start, &p, end))
            {
                return 1;
            }

            /* Unlike a normal http connection, we don't want any specific
             * timeout.  We want to stay up until the client drops us */

            lws_set_timeout(wsi, NO_PENDING_TIMEOUT, 0);

            /* write the body separately */

            lws_callback_on_writable(wsi);
        }
        break;

        case LWS_CALLBACK_HTTP_WRITEABLE:
        {
            LOG_TRACE("LWS_CALLBACK_HTTP_WRITEABLE");

//            if (!pss)
//            {
//                break;
//            }

            /*
             * to keep this demo as simple as possible, each client has his
             * own private data and timer.
             */

//            p += lws_snprintf((char*)p, end - p,
//                              "data: %llu\x0d\x0a\x0d\x0a",
//                              (unsigned long long)time(NULL) -
//                              pss->established);

//            if (lws_write(wsi, (uint8_t*)start, lws_ptr_diff(p, start),
//                          LWS_WRITE_HTTP) != lws_ptr_diff(p, start))
//            {
//                return 1;
//            }

//            lws_set_timer_usecs(wsi, 3 * LWS_USEC_PER_SEC);
        }
        break;

        case LWS_CALLBACK_CLOSED_HTTP:
            LOG_TRACE("LWS_CALLBACK_CLOSED_HTTP");
            break;

        case LWS_CALLBACK_HTTP_BODY:
            LOG_TRACE("LWS_CALLBACK_HTTP_BODY");
            break;

        case LWS_CALLBACK_HTTP_BODY_COMPLETION:
            LOG_TRACE("LWS_CALLBACK_HTTP_BODY_COMPLETION");
            break;

        default:
        {
            return lws_callback_http_dummy(wsi, reason, user, in, len);
        }
        break;
    }

    return 0;
}
}
