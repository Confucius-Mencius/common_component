#ifndef WS_THREADS_SRC_HTTP_CALLBACK_H_
#define WS_THREADS_SRC_HTTP_CALLBACK_H_

#include <libwebsockets.h>

namespace ws
{
int HTTPCallback(struct lws* wsi, enum lws_callback_reasons reason, void* user, void* in, size_t len);
}

#endif // WS_THREADS_SRC_HTTP_CALLBACK_H_
