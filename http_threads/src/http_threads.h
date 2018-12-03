#ifndef HTTP_THREADS_SRC_HTTP_THREADS_H_
#define HTTP_THREADS_SRC_HTTP_THREADS_H_

#include "http_thread_sink.h"
#include "http_threads_interface.h"
#include "module_loader.h"

namespace http
{
class Threads : public ThreadsInterface
{
public:
    Threads();
    virtual ~Threads();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ThreadsInterface /////////////////////////
    int CreateThreadGroup() override;
    ThreadGroupInterface* GetHttpThreadGroup() const override;
    void SetRelatedThreadGroup(const RelatedThreadGroup* related_thread_group) override;

private:
    int CreateHttpThreads();

private:
    ThreadsCtx threads_ctx_;
    ThreadGroupInterface* http_thread_group_;

    typedef std::vector<ThreadInterface*> HttpThreadVec;
    HttpThreadVec http_thread_vec_;

    typedef std::vector<ThreadSink*> HttpThreadSinkVec;
    HttpThreadSinkVec http_thread_sink_vec_;

    RelatedThreadGroup related_thread_group_;

    evutil_socket_t http_listen_sock_fd_;
    evutil_socket_t https_listen_sock_fd_;
};
}

#endif // HTTP_THREADS_SRC_HTTP_THREADS_H_
