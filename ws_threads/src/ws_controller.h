#ifndef WS_THREADS_SRC_WS_CONTROLLER_H_
#define WS_THREADS_SRC_WS_CONTROLLER_H_

#include <libwebsockets.h>
#include "ws_threads_interface.h"

namespace ws
{
class WSController
{
public:
    WSController();
    ~WSController();

    void SetWSThreadGroup(ThreadGroupInterface* ws_thread_group)
    {
        ws_thread_group_ = ws_thread_group;
    }

    int Initialize(const ThreadsCtx* threads_ctx);
    void Finalize();

private:
    int CreateWSContext(bool use_ssl);

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* ws_thread_group_;

    char iface_[128];
//    struct lws_http_mount http_mount_;
    char cert_file_path_[1024];
    char private_key_file_path_[1024];
    void** ws_foreign_loops_;

    // ws
    struct lws_context_creation_info ws_info_;
    struct lws_context* ws_context_;

    // wss
    struct lws_context_creation_info wss_info_;
    struct lws_context* wss_context_;
};
}

#endif // WS_THREADS_SRC_WS_CONTROLLER_H_
