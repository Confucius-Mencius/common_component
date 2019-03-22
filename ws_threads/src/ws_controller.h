#ifndef WS_THREADS_SRC_WS_CONTROLLER_H_
#define WS_THREADS_SRC_WS_CONTROLLER_H_

#include <libwebsockets.h>
#include "ws_threads_interface.h"

namespace ws
{
class Controller
{
public:
    Controller();
    ~Controller();

    void SetWSThreadGroup(ThreadGroupInterface* ws_thread_group)
    {
        ws_thread_group_ = ws_thread_group;
    }

    int Initialize(const ThreadsCtx* threads_ctx);
    void Finalize();

private:
    int CreateWSContext(bool security);

private:
    const ThreadsCtx* threads_ctx_;
    ThreadGroupInterface* ws_thread_group_;

    char iface_[128];
    char iface_ip_[INET_ADDRSTRLEN];
    char cert_file_path_[1024];
    char private_key_file_path_[1024];
    void** foreign_loops_; // foreign loops是每个线程独立的

    // ws
    struct lws_context_creation_info ws_info_;
    struct lws_context* ws_ctx_; // 这个context是被所有线程共用的，但event loop是每个线程独立的，回调中是什么情况？哪里需要加锁？TODO

    // wss
    struct lws_context_creation_info wss_info_;
    struct lws_context* wss_ctx_;
};
}

#endif // WS_THREADS_SRC_WS_CONTROLLER_H_
