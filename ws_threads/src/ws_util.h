#ifndef WS_THREADS_SRC_WS_UTIL_H_
#define WS_THREADS_SRC_WS_UTIL_H_

#include <libwebsockets.h>

namespace ws
{
void DumpTokens(struct lws* wsi);
}

#endif // WS_THREADS_SRC_WS_UTIL_H_
