#ifndef HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_COMMON_LOGIC_H_
#define HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_COMMON_LOGIC_H_

#include "http_msg_dispatcher.h"
#include "http_ws_logic_args.h"
#include "http_ws_logic_interface.h"
#include "http_ws_msg_dispatcher.h"
#include "module_loader.h"
#include "part_msg_mgr.h"
#include "proto_msg_codec.h"
#include "raw_tcp_logic_interface.h"
#include "scheduler.h"
#include "the_http_parser.h"
#include "the_ws_parser.h"

namespace tcp
{
namespace raw
{
struct HTTPWSLogicItem
{
    std::string logic_so_path;
    ModuleLoader logic_loader;
    tcp::http_ws::LogicInterface* logic;

    HTTPWSLogicItem() : logic_so_path(), logic_loader()
    {
        logic = nullptr;
    }
};

typedef std::vector<HTTPWSLogicItem> HTTPWSLogicItemVec;

class HTTPWSCommonLogic : public CommonLogicInterface, public TimerSinkInterface
{
public:
    HTTPWSCommonLogic();
    virtual ~HTTPWSCommonLogic();

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

    void OnHTTPReq(ConnID conn_id, const tcp::http_ws::http::Req& http_req);
    void OnUpgrade(ConnID conn_id, const tcp::http_ws::http::Req& http_req, const char* data, size_t len);
    void OnWSMsg(ConnID conn_id, int opcode, const char* data, size_t len);

private:
    int LoadHTTPWSCommonLogic();
    int LoadHTTPWSLogicGroup();

private:
    struct HTTPConnCtx
    {
        ConnInterface* conn;
        tcp::http_ws::http::Parser http_parser;
        bool upgrade_;
        tcp::http_ws::ws::Parser ws_parser;

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
//    void ProcessWSData(HTTPConnCtx* http_conn_ctx);

private:
    HTTPWSLogicArgs http_ws_logic_args_;

    ModuleLoader http_ws_common_logic_loader_;
    tcp::http_ws::CommonLogicInterface* http_ws_common_logic_;
    HTTPWSLogicItemVec http_ws_logic_item_vec_;

    ::proto::MsgCodec msg_codec_;
    tcp::http_ws::Scheduler scheduler_;
    tcp::http_ws::MsgDispatcher msg_dispatcher_; // proto msg dispatcher
    tcp::http_ws::http::MsgDispatcher http_msg_dispatcher_;

    tcp::http_ws::PartMsgMgr part_msg_mgr_;

    HTTPConnCtxMap http_conn_ctx_map_;
};
}
}

#endif // HTTP_WS_THREADS_SRC_RAW_TCP_COMMON_LOGIC_COMMON_LOGIC_H_
