#ifndef WS_THREADS_SRC_WS_CALLBACK_H_
#define WS_THREADS_SRC_WS_CALLBACK_H_

#include <libwebsockets.h>

namespace ws
{
/* one of these is created for each vhost our protocol is used with */
struct per_vhost_data
{
    struct lws_context* context;
    struct lws_vhost* vhost;
    const struct lws_protocols* protocol;
};

int WSCallback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);
}

#endif // WS_THREADS_SRC_WS_CALLBACK_H_
