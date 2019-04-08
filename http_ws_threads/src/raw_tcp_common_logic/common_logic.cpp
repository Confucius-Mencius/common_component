#include "common_logic.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "file_util.h"
#include "log_util.h"
#include "mem_util.h"
#include "raw_tcp_scheduler_interface.h"

namespace tcp
{
namespace raw
{
enum
{
    EXIT_CHECK_TIMER_ID = 1,
};

HTTPWSCommonLogic::HTTPWSCommonLogic() : http_ws_logic_args_(), http_ws_common_logic_loader_(),
    http_ws_logic_item_vec_(), msg_codec_(), scheduler_(), msg_dispatcher_(), part_msg_mgr_()
{
    http_ws_common_logic_ = nullptr;
}

HTTPWSCommonLogic::~HTTPWSCommonLogic()
{
}

const char* HTTPWSCommonLogic::GetVersion() const
{
    return nullptr;
}

const char* HTTPWSCommonLogic::GetLastErrMsg() const
{
    return nullptr;
}

void HTTPWSCommonLogic::Release()
{
    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        SAFE_RELEASE_MODULE(it->logic, it->logic_loader);
    }

    http_ws_logic_item_vec_.clear();
    SAFE_RELEASE_MODULE(http_ws_common_logic_, http_ws_common_logic_loader_);

    delete this;
}

int HTTPWSCommonLogic::Initialize(const void* ctx)
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

    scheduler_.SetRawTCPScheduler(logic_ctx_.scheduler);
    scheduler_.SetMsgCodec(&msg_codec_);

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

void HTTPWSCommonLogic::Finalize()
{
    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        SAFE_FINALIZE(it->logic);
    }

    SAFE_FINALIZE(http_ws_common_logic_);
    part_msg_mgr_.Finalize();
}

int HTTPWSCommonLogic::Activate()
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

void HTTPWSCommonLogic::Freeze()
{
    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        SAFE_FREEZE(it->logic);
    }

    SAFE_FREEZE(http_ws_common_logic_);

    part_msg_mgr_.Freeze();
}

void HTTPWSCommonLogic::OnStop()
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

void HTTPWSCommonLogic::OnReload()
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

void HTTPWSCommonLogic::OnClientConnected(const ConnGUID* conn_guid)
{
    LOG_TRACE("HTTPWSCommonLogic::OnClientConnected, " << conn_guid);

    ConnInterface* conn = logic_ctx_.conn_center->GetConnByID(conn_guid->conn_id);
    if (nullptr == conn)
    {
        LOG_ERROR("failed to get conn by id: " << conn_guid);
        return;
    }

    conn->ClearData();

    if (http_ws_common_logic_ != nullptr)
    {
        http_ws_common_logic_->OnClientConnected(conn_guid);
    }

    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        it->logic->OnClientConnected(conn_guid);
    }
}

void HTTPWSCommonLogic::OnClientClosed(const ConnGUID* conn_guid)
{
    LOG_TRACE("HTTPWSCommonLogic::OnClientClosed, " << conn_guid);

    ConnInterface* conn = logic_ctx_.conn_center->GetConnByID(conn_guid->conn_id);
    if (nullptr == conn)
    {
        LOG_ERROR("failed to get conn by id: " << conn_guid);
        return;
    }

    conn->ClearData();

    if (http_ws_common_logic_ != nullptr)
    {
        http_ws_common_logic_->OnClientClosed(conn_guid);
    }

    for (HTTPWSLogicItemVec::iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        it->logic->OnClientClosed(conn_guid);
    }
}

void HTTPWSCommonLogic::OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len)
{
//    LOG_DEBUG("HTTPWSCommonLogic::OnRecvClientData, " << conn_guid << ", data: " << data << ", len: " << len);

//    ConnInterface* conn = logic_ctx_.conn_center->GetConnByID(conn_guid->conn_id);
//    if (nullptr == conn)
//    {
//        LOG_ERROR("failed to get conn by id, " << conn_guid);
//        return;
//    }

//    std::string& d = conn->AppendData((const char*) data, len);
//    const char* dp = d.data();
//    const size_t dl = d.size();

//    ::proto::MsgHead msg_head;
//    ::proto::MsgID err_msg_id = MSG_ID_OK;
//    size_t total_msg_len = 0;

//    if (!msg_codec_.IsWholeMsg(err_msg_id, total_msg_len, dp, dl))
//    {
//        if (err_msg_id != MSG_ID_NOT_A_WHOLE_MSG)
//        {
//            conn->ClearData();

//            msg_head.Reset();
//            msg_head.msg_id = err_msg_id;

//            scheduler_.SendToClient(conn_guid, msg_head, nullptr, 0);

//            LOG_INFO("close proto tcp conn, " << conn_guid << ", err msg id: " << err_msg_id);
//            scheduler_.CloseClient(conn_guid); // 服务器主动关闭连接

//            return;
//        }

//        // 将该client加入一个按上一次接收到不完整消息的时间升序排列的列表,收到完整消息则从列表中移除.如果一段时间后任没有收到完整消息,则主动关闭连接
//        part_msg_mgr_.UpsertRecord(conn, *conn_guid, http_ws_logic_args_.app_frame_conf_mgr->GetProtoPartMsgConnLife());
//        return;
//    }

//    part_msg_mgr_.RemoveRecord(conn);

//    char* msg_body = nullptr;
//    size_t msg_body_len = 0;

//    msg_head.Reset();

//    if (msg_codec_.DecodeMsg(err_msg_id, &msg_head, &msg_body, msg_body_len, dp + TOTAL_MSG_LEN_FIELD_LEN, total_msg_len) != 0)
//    {
//        msg_head.Reset();
//        msg_head.msg_id = err_msg_id;

//        scheduler_.SendToClient(conn_guid, msg_head, nullptr, 0);
//        return;
//    }

//    OnRecvClientMsg(conn->GetConnGUID(), msg_head, msg_body, msg_body_len);

//    const size_t left = dl - TOTAL_MSG_LEN_FIELD_LEN - total_msg_len;
//    if (left > 0)
//    {
//        d.assign(dp + total_msg_len + total_msg_len, left); // TODO 重叠assign是否安全？
//    }
}

void HTTPWSCommonLogic::OnTask(const ConnGUID* conn_guid, ThreadInterface* source_thread, const void* data, size_t len)
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
        LOG_TRACE("dispatch msg ok, " << conn_guid << ", msg id: " << msg_head.msg_id);
        return;
    }
}

void HTTPWSCommonLogic::OnTimer(TimerID timer_id, void* data, size_t len, int times)
{
    int can_exit = 1;

    if (http_ws_common_logic_ != nullptr)
    {
        can_exit &= (http_ws_common_logic_->CanExit() ? 1 : 0);
    }

    for (HTTPWSLogicItemVec::const_iterator it = http_ws_logic_item_vec_.begin(); it != http_ws_logic_item_vec_.end(); ++it)
    {
        can_exit &= (it->logic->CanExit() ? 1 : 0);
    }

    if (can_exit != 0)
    {
        can_exit_ = true;
    }
}

int HTTPWSCommonLogic::LoadHTTPWSCommonLogic()
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

int HTTPWSCommonLogic::LoadHTTPWSLogicGroup()
{
    // logic so group
    if (0 == http_ws_logic_args_.app_frame_conf_mgr->GetProtoTCPLogicSoGroup().size())
    {
        return 0;
    }

    const StrGroup logic_so_group = http_ws_logic_args_.app_frame_conf_mgr->GetProtoTCPLogicSoGroup();

    for (StrGroup::const_iterator it = logic_so_group.begin(); it != logic_so_group.end(); ++it)
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

void HTTPWSCommonLogic::OnRecvClientMsg(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
{
//    if (http_ws_logic_item_vec_.size() > 0)
//    {
//        // 内置心跳消息处理
//        if (MSG_ID_HEARTBEAT_REQ == msg_head.msg_id)
//        {
//            ::proto::MsgHead rsp_msg_head = msg_head;
//            rsp_msg_head.msg_id = MSG_ID_HEARTBEAT_RSP;

//            scheduler_.SendToClient(conn_guid, rsp_msg_head, nullptr, 0);
//            return;
//        }

//        if (0 == msg_dispatcher_.DispatchMsg(conn_guid, msg_head, msg_body, msg_body_len))
//        {
//            LOG_TRACE("dispatch msg ok, " << conn_guid << ", msg id: " << msg_head.msg_id);
//            return;
//        }
//    }

//    // 没有io logic或者io logic派发失败，把任务均匀分配给work线程
//    if (nullptr == http_ws_logic_args_.related_thread_groups->work_thread_group ||
//            0 == http_ws_logic_args_.related_thread_groups->work_thread_group->GetThreadCount())
//    {
//        LOG_ERROR("no work threads, failed to dispatch msg, " << conn_guid << ", msg id: " << msg_head.msg_id);

//        ::proto::MsgHead rsp_msg_head = msg_head;
//        rsp_msg_head.msg_id = MSG_ID_NONE_HANDLER_FOUND;

//        scheduler_.SendToClient(conn_guid, rsp_msg_head, nullptr, 0);
//        return;
//    }

//    LOG_TRACE("forward to work thread");

//    if (scheduler_.SendToWorkThread(conn_guid, msg_head, msg_body, msg_body_len, -1) != 0)
//    {
//        LOG_ERROR("failed to send to work thread");

//        ::proto::MsgHead rsp_msg_head = msg_head;
//        rsp_msg_head.msg_id = MSG_ID_SCHEDULE_FAILED;

//        scheduler_.SendToClient(conn_guid, rsp_msg_head, nullptr, 0);
//        return;
//    }
}
}
}
