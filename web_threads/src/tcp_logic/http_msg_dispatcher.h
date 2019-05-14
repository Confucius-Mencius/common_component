#ifndef WEB_THREADS_SRC_TCP_LOGIC_HTTP_MSG_DISPATCHER_H_
#define WEB_THREADS_SRC_TCP_LOGIC_HTTP_MSG_DISPATCHER_H_

#include "the_http_parser.h"
#include "http_msg_handler_interface.h"

namespace tcp
{
namespace web
{
namespace http
{
class MsgDispatcher : public MsgDispatcherInterface
{
public:
    MsgDispatcher();
    virtual ~MsgDispatcher();

    ///////////////////////// MsgDispatcherInterface /////////////////////////
    int AttachMsgHandler(const char* path, MsgHandlerInterface* msg_handler) override;
    void DetachMsgHandler(const char* path) override;

    int DispatchMsg(const ConnInterface* conn, const Req& req);

private:
    typedef std::map<std::string, MsgHandlerInterface*> MsgHandlerMap; // path ->
    MsgHandlerMap msg_handler_map_;
};
}
}
}

#endif // WEB_THREADS_SRC_TCP_LOGIC_HTTP_MSG_DISPATCHER_H_
