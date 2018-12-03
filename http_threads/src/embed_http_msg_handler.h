#ifndef HTTP_THREADS_SRC_EMBED_HTTP_MSG_HANDLER_H_
#define HTTP_THREADS_SRC_EMBED_HTTP_MSG_HANDLER_H_

#include <evhttp.h>
#include "http_logic_interface.h"
#include "http_msg_handler_interface.h"

namespace http
{
class ThreadSink;

class EmbedMsgHandler : public MsgHandlerInterface
{
public:
    EmbedMsgHandler();
    virtual ~EmbedMsgHandler();

    void SetThreadSink(ThreadSink* sink)
    {
        thread_sink_ = sink;
    }

protected:
    ThreadSink* thread_sink_;
};
}

#endif // HTTP_THREADS_SRC_EMBED_HTTP_MSG_HANDLER_H_
