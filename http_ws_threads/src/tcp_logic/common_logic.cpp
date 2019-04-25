#include "common_logic.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "file_util.h"
#include "log_util.h"
#include "mem_util.h"
#include "tcp_scheduler_interface.h"

namespace tcp
{
enum
{
    EXIT_CHECK_TIMER_ID = 1,
};

HTTPWSLogic::HTTPWSLogic() : http_ws_logic_args_(), http_ws_common_logic_loader_(),
    http_ws_logic_item_vec_(), msg_codec_(), scheduler_(), msg_dispatcher_(), http_msg_dispatcher_(),
    part_msg_mgr_(), http_conn_ctx_map_()
{
    http_ws_common_logic_ = nullptr;
}

HTTPWSLogic::~HTTPWSLogic()
{
}

const char* HTTPWSLogic::GetVersion() const
{
    return nullptr;
}

const char* HTTPWSLogic::GetLastErrMsg() const
{
    return nullptr;
}

void HTTPWSLogic::Release()
{
    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        SAFE_RELEASE_MODULE(it->logic, it->logic_loader);
    }

    http_ws_logic_item_vec_.clear();
    SAFE_RELEASE_MODULE(http_ws_common_logic_, http_ws_common_logic_loader_);

    delete this;
}

int HTTPWSLogic::Initialize(const void* ctx)
{
    if (LogicInterface::Initialize(ctx) != 0)
    {
        return -1;
    }

    http_ws_logic_args_ = *(static_cast<const HTTPWSLogicArgs*>(logic_ctx_.logic_args));

    ::proto::MsgCodecCtx msg_codec_ctx;
    msg_codec_ctx.max_msg_body_len = http_ws_logic_args_.app_frame_conf_mgr->GetProtoMaxMsgBodyLen();
    msg_codec_ctx.do_checksum = http_ws_logic_args_.app_frame_conf_mgr->ProtoDoChecksum();
    msg_codec_.SetCtx(&msg_codec_ctx);

    scheduler_.SetTCPScheduler(logic_ctx_.scheduler);
    scheduler_.SetMsgCodec(&msg_codec_);

    part_msg_mgr_.SetScheduler(&scheduler_);

    if (part_msg_mgr_.Initialize(logic_ctx_.timer_axis, { http_ws_logic_args_.app_frame_conf_mgr->GetHTTPWSPartMsgCheckInterval(), 0 }) != 0)
    {
        return -1;
    }

    if (LoadHTTPWSCommonLogic() != 0)
    {
        return -1;
    }

    if (LoadHTTPWSLogicGroup() != 0)
    {
        return -1;
    }

    return 0;
}

void HTTPWSLogic::Finalize()
{
    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        SAFE_FINALIZE(it->logic);
    }

    SAFE_FINALIZE(http_ws_common_logic_);
    part_msg_mgr_.Finalize();

    for (HTTPConnCtxMap::iterator it = http_conn_ctx_map_.begin(); it != http_conn_ctx_map_.end(); ++it)
    {
        it->second->Release();
    }

    http_conn_ctx_map_.clear();
}

int HTTPWSLogic::Activate()
{
    if (part_msg_mgr_.Activate() != 0)
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(http_ws_common_logic_))
    {
        return -1;
    }

    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        if (SAFE_ACTIVATE_FAILED(it->logic))
        {
            return -1;
        }
    }

    return 0;
}

void HTTPWSLogic::Freeze()
{
    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        SAFE_FREEZE(it->logic);
    }

    SAFE_FREEZE(http_ws_common_logic_);

    part_msg_mgr_.Freeze();
}

void HTTPWSLogic::OnStop()
{
    if (http_ws_common_logic_ != nullptr)
    {
        http_ws_common_logic_->OnStop();
    }

    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        it->logic->OnStop();
    }

    // 启动定时器，检查http-ws tcp logics是否都可以退出了。100毫秒检查一次
    struct timeval tv = { 0, 100000 };

    if (logic_ctx_.timer_axis->SetTimer(this, EXIT_CHECK_TIMER_ID, tv, nullptr, 0) != 0)
    {
        LOG_ERROR("failed to start exit check timer");
        return;
    }
}

void HTTPWSLogic::OnReload()
{
    if (http_ws_common_logic_ != nullptr)
    {
        http_ws_common_logic_->OnReload();
    }

    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        it->logic->OnReload();
    }
}

void HTTPWSLogic::OnClientConnected(const ConnGUID* conn_guid)
{
    LOG_TRACE("HTTPWSCommonLogic::OnClientConnected, " << *conn_guid);

    ConnInterface* conn = logic_ctx_.conn_center->GetConnByID(conn_guid->conn_id);
    if (nullptr == conn)
    {
        LOG_ERROR("failed to get conn by id: " << conn_guid->conn_id);
        return;
    }

    HTTPConnCtx* http_conn_ctx = HTTPConnCtx::Create();
    if (nullptr == http_conn_ctx)
    {
        LOG_ERROR("failed to alloc memory");
        return;
    }

    http_conn_ctx->conn = conn;
    http_conn_ctx->http_parser.SetTCPCommonLogic(this);
    http_conn_ctx->http_parser.SetConnID(conn_guid->conn_id);
    http_conn_ctx->ws_parser.SetTCPCommonLogic(this);
    http_conn_ctx->ws_parser.SetConnID(conn_guid->conn_id);

    if (!http_conn_ctx_map_.insert(HTTPConnCtxMap::value_type(conn_guid->conn_id, http_conn_ctx)).second)
    {
        LOG_ERROR("failed to insert to map, conn id: " << conn_guid->conn_id);
        return;
    }

    if (http_ws_common_logic_ != nullptr)
    {
        http_ws_common_logic_->OnClientConnected(conn_guid);
    }

    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        it->logic->OnClientConnected(conn_guid);
    }
}

void HTTPWSLogic::OnClientClosed(const ConnGUID* conn_guid)
{
    LOG_TRACE("HTTPWSCommonLogic::OnClientClosed, " << *conn_guid);

    ConnInterface* conn = logic_ctx_.conn_center->GetConnByID(conn_guid->conn_id);
    if (nullptr == conn)
    {
        LOG_ERROR("failed to get conn by id: " << conn_guid->conn_id);
        return;
    }

    if (http_ws_common_logic_ != nullptr)
    {
        http_ws_common_logic_->OnClientClosed(conn_guid);
    }

    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        it->logic->OnClientClosed(conn_guid);
    }

    HTTPConnCtxMap::iterator it = http_conn_ctx_map_.find(conn_guid->conn_id);
    if (it != http_conn_ctx_map_.end())
    {
        it->second->Release();
        http_conn_ctx_map_.erase(conn_guid->conn_id);
    }

    part_msg_mgr_.RemoveRecord(conn);
}

void HTTPWSLogic::OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len)
{
    // TODO https wss
    LOG_DEBUG("HTTPWSCommonLogic::OnRecvClientData, " << *conn_guid << ", data: " << data << ", len: " << len);

    HTTPConnCtxMap::iterator it = http_conn_ctx_map_.find(conn_guid->conn_id);
    if (it == http_conn_ctx_map_.end())
    {
        LOG_ERROR("failed to find conn by id: " << conn_guid->conn_id);
        return;
    }

    HTTPConnCtx* http_conn_ctx = it->second;

    if (!http_conn_ctx->upgrade_)
    {
        http_conn_ctx->http_parser.Execute((const char*) data, len);
    }
    else
    {
        http_conn_ctx->ws_parser.Execute((const char*) data, len);
    }
}

void HTTPWSLogic::OnTask(const ConnGUID* conn_guid, ThreadInterface* source_thread, const void* data, size_t len)
{
    if (nullptr == data || 0 == len)
    {
        LOG_ERROR("invalid params");
        return;
    }

    ::proto::MsgID err_msg_id;
    ::proto::MsgHead msg_head;
    char* msg_body = nullptr;
    size_t msg_body_len = 0;

    if (msg_codec_.DecodeMsg(err_msg_id, &msg_head, &msg_body, msg_body_len, (const char*) data, len) != 0)
    {
        return;
    }

    if (0 == msg_dispatcher_.DispatchMsg(conn_guid, msg_head, msg_body, msg_body_len))
    {
        if (conn_guid != nullptr)
        {
            LOG_TRACE("dispatch msg ok, " << *conn_guid << ", msg id: " << msg_head.msg_id);
        }
        else
        {
            LOG_TRACE("dispatch msg ok, msg id: " << msg_head.msg_id);
        }
    }
    else
    {
        LOG_ERROR("failed to dispatch msg, msg id: " << msg_head.msg_id);
        return;
    }
}

void HTTPWSLogic::OnTimer(TimerID timer_id, void* data, size_t len, int times)
{
    int can_exit = 1;

    if (http_ws_common_logic_ != nullptr)
    {
        can_exit &= (http_ws_common_logic_->CanExit() ? 1 : 0);
    }

    for (HTTPWSLogicItemVec::const_iterator it = http_ws_logic_item_vec_.cbegin(); it != http_ws_logic_item_vec_.cend(); ++it)
    {
        can_exit &= (it->logic->CanExit() ? 1 : 0);
    }

    if (can_exit != 0)
    {
        can_exit_ = true;
    }
}

void HTTPWSLogic::OnHTTPReq(ConnID conn_id, const tcp::http_ws::http::Req& http_req)
{
    ConnInterface* conn = logic_ctx_.conn_center->GetConnByID(conn_id);
    if (nullptr == conn)
    {
        LOG_ERROR("failed to get conn by id: " << conn_id);
        return;
    }

    part_msg_mgr_.RemoveRecord(conn);

    if (http_msg_dispatcher_.DispatchMsg(conn, http_req) != 0)
    {
        LOG_ERROR("failed to dispatch http req, path: " << http_req.Path << ", method: " << http_method_str(http_req.Method));
        return;
    }

    LOG_TRACE("dispatch http req ok, path: " << http_req.Path << ", method: " << http_method_str(http_req.Method));
}

void HTTPWSLogic::OnUpgrade(ConnID conn_id, const tcp::http_ws::http::Req& http_req, const char* data, size_t len)
{
    HTTPConnCtxMap::iterator it = http_conn_ctx_map_.find(conn_id);
    if (it == http_conn_ctx_map_.end())
    {
        LOG_ERROR("failed to find conn by id: " << conn_id);
        return;
    }

    HTTPConnCtx* http_conn_ctx = it->second;
    ConnInterface* conn = http_conn_ctx->conn;
    const ConnGUID* conn_guid = conn->GetConnGUID();

    // 检查upgrade头，不符合标准则关闭连接
    if (http_conn_ctx->ws_parser.CheckUpgrade(http_req) != 0)
    {
        scheduler_.CloseClient(conn_guid); // 服务器主动关闭连接
        return;
    }

    // 回复握手信息
    const std::string handshake = http_conn_ctx->ws_parser.MakeHandshake();
    LOG_DEBUG("handshake =>\n" << handshake);

    if (scheduler_.SendToClient(conn_guid, handshake.data(), handshake.size()) != 0)
    {
        scheduler_.CloseClient(conn_guid);
        return;
    }

    LOG_TRACE("send handshake ok, " << *conn_guid);
    http_conn_ctx->upgrade_ = true;

    LOG_DEBUG("len: " << len);
    if (len > 0)
    {
        LOG_DEBUG("data: " << data << ", len: " << len);
        http_conn_ctx->ws_parser.Execute(data, len);
    }
}

void HTTPWSLogic::OnWSMsg(ConnID conn_id, int opcode, const char* data, size_t len)
{
    HTTPConnCtxMap::iterator it = http_conn_ctx_map_.find(conn_id);
    if (it == http_conn_ctx_map_.end())
    {
        LOG_ERROR("failed to find conn by id: " << conn_id);
        return;
    }

    HTTPConnCtx* http_conn_ctx = it->second;
    ConnInterface* conn = http_conn_ctx->conn;
    const ConnGUID* conn_guid = conn->GetConnGUID();

    part_msg_mgr_.RemoveRecord(conn);

    // TODO 完善websocket cs消息交互流程
    switch (opcode)
    {
        case WS_OP_TEXT:
        case WS_OP_BINARY:
        {
            const tcp::http_ws::ws::FrameType frame_type = (WS_OP_TEXT == opcode) ? tcp::http_ws::ws::TEXT_FRAME : tcp::http_ws::ws::BINARY_FRAME;

            for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
            {
                it->logic->OnWSMsg(conn_guid, frame_type, data, len);
            }
        }
        break;

        case WS_OP_CLOSE:
        {
            const int flags = WS_OP_CLOSE | WS_FINAL_FRAME;
            const size_t frame_len = websocket_calc_frame_size((websocket_flags) flags, 0);

            char* frame = (char*) malloc(frame_len);
            if (nullptr == frame)
            {
                LOG_ERROR("failed to alloc memory");
                return;
            }

            websocket_build_frame(frame, (websocket_flags) flags, nullptr, nullptr, 0);

            if (0 == scheduler_.SendToClient(conn_guid, frame, frame_len))
            {
                LOG_TRACE("send close frame ok, " << *conn_guid);
            }

            free(frame);
        }
        break;

        case WS_OP_PING:
        {
            const int flags = WS_OP_PONG | WS_FINAL_FRAME;
            const size_t frame_len = websocket_calc_frame_size((websocket_flags) flags, len);

            char* frame = (char*) malloc(frame_len);
            if (nullptr == frame)
            {
                LOG_ERROR("failed to alloc memory");
                return;
            }

            websocket_build_frame(frame, (websocket_flags) flags, nullptr, data, len);

            if (0 == scheduler_.SendToClient(conn_guid, frame, frame_len))
            {
                LOG_TRACE("send pong ok, " << *conn_guid);
            }

            free(frame);
        }
        break;

        case WS_OP_PONG:
        {
            // 什么都不做(丢弃内容)
        }
        break;

        default:
        {
        }
        break;
    }
}

void HTTPWSLogic::RecordPartMsg(ConnID conn_id)
{
    ConnInterface* conn = logic_ctx_.conn_center->GetConnByID(conn_id);
    if (nullptr == conn)
    {
        LOG_ERROR("failed to get conn by id: " << conn_id);
        return;
    }

    // 将该client加入一个按上一次接收到不完整消息的时间升序排列的列表,收到完整消息则从列表中移除.如果一段时间后任没有收到完整消息,则主动关闭连接
    part_msg_mgr_.UpsertRecord(conn, *conn->GetConnGUID(), http_ws_logic_args_.app_frame_conf_mgr->GetHTTPWSPartMsgConnLife());
}

int HTTPWSLogic::LoadHTTPWSCommonLogic()
{
    const std::string common_logic_so = http_ws_logic_args_.app_frame_conf_mgr->GetHTTPWSCommonLogicSo();
    if (0 == common_logic_so.length())
    {
        return 0;
    }

    char common_logic_so_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(common_logic_so_path, sizeof(common_logic_so_path),
                    common_logic_so.c_str(), logic_ctx_.cur_working_dir);
    LOG_ALWAYS("load common logic so " << common_logic_so_path << " begin");

    if (http_ws_common_logic_loader_.Load(common_logic_so_path) != 0)
    {
        LOG_ERROR("failed to load common logic so, " << http_ws_common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    http_ws_common_logic_ = static_cast<tcp::http_ws::CommonLogicInterface*>(http_ws_common_logic_loader_.GetModuleInterface());
    if (nullptr == http_ws_common_logic_)
    {
        LOG_ERROR("failed to get common logic, " << http_ws_common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    tcp::http_ws::LogicCtx logic_ctx;
    logic_ctx.argc = logic_ctx_.argc;
    logic_ctx.argv = logic_ctx_.argv;
    logic_ctx.common_component_dir = logic_ctx_.common_component_dir;
    logic_ctx.cur_working_dir = logic_ctx_.cur_working_dir;
    logic_ctx.app_name = logic_ctx_.app_name;
    logic_ctx.conf_center = logic_ctx_.conf_center;
    logic_ctx.timer_axis = logic_ctx_.timer_axis;
    logic_ctx.conn_center = logic_ctx_.conn_center;
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.msg_dispatcher = &msg_dispatcher_;
    logic_ctx.http_msg_dispatcher = &http_msg_dispatcher_;
    logic_ctx.common_logic = http_ws_common_logic_;
    logic_ctx.thread_ev_base = logic_ctx_.thread_ev_base;
    logic_ctx.thread_idx = logic_ctx_.thread_idx;

    if (http_ws_common_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_ALWAYS("load common logic so " << common_logic_so_path << " end");
    return 0;
}

int HTTPWSLogic::LoadHTTPWSLogicGroup()
{
    // logic so group
    if (0 == http_ws_logic_args_.app_frame_conf_mgr->GetHTTPWSLogicSoGroup().size())
    {
        return 0;
    }

    const StrGroup logic_so_group = http_ws_logic_args_.app_frame_conf_mgr->GetHTTPWSLogicSoGroup();

    for (StrGroup::const_iterator it = logic_so_group.cbegin(); it != logic_so_group.cend(); ++it)
    {
        char logic_so_path[MAX_PATH_LEN] = "";
        GetAbsolutePath(logic_so_path, sizeof(logic_so_path), (*it).c_str(), logic_ctx_.cur_working_dir);

        HTTPWSLogicItem logic_item;
        logic_item.logic_so_path = logic_so_path;
        logic_item.logic = nullptr;

        http_ws_logic_item_vec_.push_back(logic_item);
    }

    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        HTTPWSLogicItem& logic_item = *it;
        LOG_ALWAYS("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = static_cast<tcp::http_ws::LogicInterface*>(logic_item.logic_loader.GetModuleInterface());
        if (nullptr == logic_item.logic)
        {
            LOG_ERROR("failed to get logic, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        tcp::http_ws::LogicCtx logic_ctx;
        logic_ctx.argc = logic_ctx_.argc;
        logic_ctx.argv = logic_ctx_.argv;
        logic_ctx.common_component_dir = logic_ctx_.common_component_dir;
        logic_ctx.cur_working_dir = logic_ctx_.cur_working_dir;
        logic_ctx.app_name = logic_ctx_.app_name;
        logic_ctx.conf_center = logic_ctx_.conf_center;
        logic_ctx.timer_axis = logic_ctx_.timer_axis;
        logic_ctx.conn_center = logic_ctx_.conn_center;
        logic_ctx.scheduler = &scheduler_;
        logic_ctx.msg_dispatcher = &msg_dispatcher_;
        logic_ctx.http_msg_dispatcher = &http_msg_dispatcher_;
        logic_ctx.common_logic = http_ws_common_logic_;
        logic_ctx.thread_ev_base = logic_ctx_.thread_ev_base;
        logic_ctx.thread_idx = logic_ctx_.thread_idx;

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_ALWAYS("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}
}
