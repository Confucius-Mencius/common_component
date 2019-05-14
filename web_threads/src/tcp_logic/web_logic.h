#ifndef WEB_THREADS_SRC_TCP_LOGIC_WEB_LOGIC_H_
#define WEB_THREADS_SRC_TCP_LOGIC_WEB_LOGIC_H_

#include "http_msg_dispatcher.h"
#include "web_logic_args.h"
#include "web_logic_interface.h"
#include "web_msg_dispatcher.h"
#include "module_loader.h"
#include "part_msg_mgr.h"
#include "proto_msg_codec.h"
#include "scheduler.h"
#include "tcp_logic_interface.h"
#include "the_http_parser.h"
#include "the_ws_parser.h"

namespace tcp
{
struct WebLogicItem
{
    std::string logic_so_path;
    ModuleLoader logic_loader;
    tcp::web::LogicInterface* logic;

    WebLogicItem() : logic_so_path(), logic_loader()
    {
        logic = nullptr;
    }
};

typedef std::vector<WebLogicItem> WebLogicItemVec;

class WebLogic : public LogicInterface, public TimerSinkInterface
{
public:
    WebLogic();
    virtual ~WebLogic();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// tcp::LogicInterface /////////////////////////
    void OnStop() override;
    void OnReload() override;
    void OnClientConnected(const ConnGUID* conn_guid) override;
    void OnClientClosed(const ConnGUID* conn_guid) override;
    void OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len) override;
    void OnTask(const ConnGUID* conn_guid, ThreadInterface* source_thread, const void* data, size_t len) override;

    ///////////////////////// TimerSinkInterface /////////////////////////
    void OnTimer(TimerID timer_id, void* data, size_t len, int times);

    void OnHTTPReq(bool& conn_closed, ConnID conn_id, const tcp::web::http::Req& http_req);
    void OnUpgrade(ConnID conn_id, const tcp::web::http::Req& http_req, const char* data, size_t len);
    void OnWSMsg(bool& conn_closed, ConnID conn_id, int opcode, const char* data, size_t len);

    void RecordPartMsg(ConnID conn_id);

private:
    int LoadWebCommonLogic();
    int LoadWebLogicGroup();

private:
    struct HTTPConnCtx
    {
        ConnInterface* conn;
        tcp::web::http::Parser http_parser;
        bool upgrade_;
        tcp::web::ws::Parser ws_parser;

        HTTPConnCtx() : http_parser(), ws_parser()
        {
            conn = nullptr;
            upgrade_ = false;
        }

        static HTTPConnCtx* Create()
        {
            return new HTTPConnCtx();
        }

        void Release()
        {
            delete this;
        }
    };

    typedef __hash_map<ConnID, HTTPConnCtx*> HTTPConnCtxMap;

private:
    WebLogicArgs web_logic_args_;

    ModuleLoader web_common_logic_loader_;
    tcp::web::CommonLogicInterface* web_common_logic_;
    WebLogicItemVec web_logic_item_vec_;

    ::proto::MsgCodec msg_codec_;
    tcp::web::Scheduler scheduler_;
    tcp::web::MsgDispatcher msg_dispatcher_; // proto msg dispatcher
    tcp::web::http::MsgDispatcher http_msg_dispatcher_;

    tcp::web::PartMsgMgr part_msg_mgr_;

    HTTPConnCtxMap http_conn_ctx_map_;
};
}

#endif // WEB_THREADS_SRC_TCP_LOGIC_WEB_LOGIC_H_
