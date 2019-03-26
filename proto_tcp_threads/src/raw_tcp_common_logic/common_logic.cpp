#include "common_logic.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "log_util.h"
#include "file_util.h"
#include "raw_tcp_scheduler_interface.h"

namespace tcp
{
namespace raw
{
ProtoCommonLogic::ProtoCommonLogic() : proto_tcp_logic_args_(), proto_tcp_common_logic_loader_(),
    proto_tcp_logic_item_vec_(), part_msg_mgr_(), msg_dispatcher_()
{
    proto_tcp_common_logic_ = NULL;
}

ProtoCommonLogic::~ProtoCommonLogic()
{
}

const char* ProtoCommonLogic::GetVersion() const
{
    return NULL;
}

const char* ProtoCommonLogic::GetLastErrMsg() const
{
    return NULL;
}

void ProtoCommonLogic::Release()
{
    delete this;
}

int ProtoCommonLogic::Initialize(const void* ctx)
{
    if (LogicInterface::Initialize(ctx) != 0)
    {
        return -1;
    }

    proto_tcp_logic_args_ = *(static_cast<const ProtoArgs*>(logic_ctx_.logic_args));
    scheduler_.SetRawTCPScheduler(logic_ctx_.scheduler);
    scheduler_.SetMsgCodec(proto_tcp_logic_args_.proto_msg_codec);

    if (LoadProtoTCPCommonLogic() != 0)
    {
        return -1;
    }

    if (LoadProtoTCPLogicGroup() != 0)
    {
        return -1;
    }

    return 0;
}

void ProtoCommonLogic::Finalize()
{
}

int ProtoCommonLogic::Activate()
{
    return 0;
}

void ProtoCommonLogic::Freeze()
{
}

void ProtoCommonLogic::OnStop()
{
    can_exit_ = true;
}

void ProtoCommonLogic::OnReload()
{
}

void ProtoCommonLogic::OnClientConnected(const ConnGUID* conn_guid)
{
    LOG_TRACE("conn connected, " << *conn_guid);

    ConnInterface* conn = logic_ctx_.conn_center->GetConnByID(conn_guid->conn_id);
    if (NULL == conn)
    {
        LOG_ERROR("failed to get conn by id: " << conn_guid);
        return;
    }

    conn->ClearData();
}

void ProtoCommonLogic::OnClientClosed(const ConnGUID* conn_guid)
{
    LOG_TRACE("conn closed, " << *conn_guid);

    ConnInterface* conn = logic_ctx_.conn_center->GetConnByID(conn_guid->conn_id);
    if (NULL == conn)
    {
        LOG_ERROR("failed to get conn by id: " << conn_guid);
        return;
    }

    conn->ClearData();
}

void ProtoCommonLogic::OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len)
{
    LOG_DEBUG(*conn_guid << ", data: " << data << ", len: " << len);

    ConnInterface* conn = logic_ctx_.conn_center->GetConnByID(conn_guid->conn_id);
    if (NULL == conn)
    {
        LOG_ERROR("failed to get conn by id, " << conn_guid);
        return;
    }

    std::string& d = conn->AppendData((const char*) data, len);
    const char* dp = d.data();
    const size_t dl = d.size();

    ::proto::MsgCodecInterface* msg_codec = proto_tcp_logic_args_.proto_msg_codec;

    ::proto::MsgHead msg_head;
    ::proto::MsgID err_msg_id = MSG_ID_OK;
    size_t total_msg_len = 0;

    if (!msg_codec->IsWholeMsg(err_msg_id, total_msg_len, dp, dl))
    {
        if (err_msg_id != MSG_ID_NOT_A_WHOLE_MSG)
        {
            conn->ClearData();

            msg_head.Reset();
            msg_head.msg_id = err_msg_id;

            scheduler_.SendToClient(conn_guid, msg_head, NULL, 0);

            LOG_INFO("close proto tcp conn, " << *conn_guid << ", err msg id: " << err_msg_id);
            scheduler_.CloseClient(conn_guid); // 服务器主动关闭连接

            return;
        }

        // 将该client加入一个按上一次接收到不完整消息的时间升序排列的列表,收到完整消息则从列表中移除.如果一段时间后任没有收到完整消息,则主动关闭连接
        part_msg_mgr_.UpsertRecord(conn, *conn_guid, proto_tcp_logic_args_.app_frame_conf_mgr->GetProtoPartMsgConnLife());
        return;
    }

    part_msg_mgr_.RemoveRecord(conn);

    char* msg_body = NULL;
    size_t msg_body_len = 0;

    msg_head.Reset();

    if (msg_codec->DecodeMsg(err_msg_id, &msg_head, &msg_body, msg_body_len, dp + TOTAL_MSG_LEN_FIELD_LEN, total_msg_len) != 0)
    {
        msg_head.Reset();
        msg_head.msg_id = err_msg_id;

        scheduler_.SendToClient(conn_guid, msg_head, NULL, 0);
        return;
    }

//    OnRecvClientMsg(&conn->GetConnGuid(), msg_head, msg_body, msg_body_len);

    const size_t left = dl - TOTAL_MSG_LEN_FIELD_LEN - total_msg_len;
    if (left > 0)
    {
        d.assign(dp + total_msg_len + total_msg_len, left); // TODO 重叠assign是否安全？
    }
}

void ProtoCommonLogic::OnTask(const ConnGUID* conn_guid, ThreadInterface* source_thread, const void* data, size_t len)
{
    (void) conn_guid;
    (void) source_thread;
    (void) data;
    (void) len;
}

int ProtoCommonLogic::LoadProtoTCPCommonLogic()
{
    if (0 == proto_tcp_logic_args_.app_frame_conf_mgr->GetProtoTCPCommonLogicSo().length())
    {
        return 0;
    }

    const std::string& proto_tcp_common_logic_so = proto_tcp_logic_args_.app_frame_conf_mgr->GetProtoTCPCommonLogicSo();
    if (0 == proto_tcp_common_logic_so.length())
    {
        return 0;
    }

    char proto_tcp_common_logic_so_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(proto_tcp_common_logic_so_path, sizeof(proto_tcp_common_logic_so_path),
                    proto_tcp_common_logic_so.c_str(), logic_ctx_.cur_working_dir);
    LOG_TRACE("load common logic so " << proto_tcp_common_logic_so_path << " begin");

    if (proto_tcp_common_logic_loader_.Load(proto_tcp_common_logic_so_path) != 0)
    {
        LOG_ERROR("failed to load common logic so, " << proto_tcp_common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    proto_tcp_common_logic_ = static_cast<tcp::proto::CommonLogicInterface*>(proto_tcp_common_logic_loader_.GetModuleInterface());
    if (NULL == proto_tcp_common_logic_)
    {
        LOG_ERROR("failed to get common logic, " << proto_tcp_common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    tcp::proto::LogicCtx logic_ctx;
    logic_ctx.argc = logic_ctx_.argc;
    logic_ctx.argv = logic_ctx_.argv;
    logic_ctx.common_component_dir = logic_ctx_.common_component_dir;
    logic_ctx.cur_working_dir = logic_ctx_.cur_working_dir;
    logic_ctx.app_name = logic_ctx_.app_name;
    logic_ctx.conf_center = logic_ctx_.conf_center;
    logic_ctx.timer_axis = logic_ctx_.timer_axis;
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.common_logic = proto_tcp_common_logic_;
    logic_ctx.thread_ev_base = logic_ctx_.thread_ev_base;

    if (proto_tcp_common_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_TRACE("load common logic so " << proto_tcp_common_logic_so_path << " end");
    return 0;
}

int ProtoCommonLogic::LoadProtoTCPLogicGroup()
{
    // logic so group
    if (0 == proto_tcp_logic_args_.app_frame_conf_mgr->GetProtoTCPLogicSoGroup().size())
    {
        return 0;
    }

    LogicItem proto_tcp_logic_item;
    proto_tcp_logic_item.logic = NULL;

    const StrGroup& proto_tcp_logic_so_group = proto_tcp_logic_args_.app_frame_conf_mgr->GetProtoTCPLogicSoGroup();

    for (StrGroup::const_iterator it = proto_tcp_logic_so_group.begin();
            it != proto_tcp_logic_so_group.end(); ++it)
    {
        char logic_so_path[MAX_PATH_LEN] = "";
        GetAbsolutePath(logic_so_path, sizeof(logic_so_path), (*it).c_str(), logic_ctx_.cur_working_dir);
        proto_tcp_logic_item.logic_so_path = logic_so_path;
        proto_tcp_logic_item_vec_.push_back(proto_tcp_logic_item);
    }

    for (LogicItemVec::iterator it = proto_tcp_logic_item_vec_.begin(); it != proto_tcp_logic_item_vec_.end(); ++it)
    {
        LogicItem& logic_item = *it;
        LOG_TRACE("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = static_cast<tcp::proto::LogicInterface*>(logic_item.logic_loader.GetModuleInterface());
        if (NULL == logic_item.logic)
        {
            LOG_ERROR("failed to get logic, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        tcp::proto::LogicCtx logic_ctx;
        logic_ctx.argc = logic_ctx_.argc;
        logic_ctx.argv = logic_ctx_.argv;
        logic_ctx.common_component_dir = logic_ctx_.common_component_dir;
        logic_ctx.cur_working_dir = logic_ctx_.cur_working_dir;
        logic_ctx.app_name = logic_ctx_.app_name;
        logic_ctx.conf_center = logic_ctx_.conf_center;
        logic_ctx.timer_axis = logic_ctx_.timer_axis;
        logic_ctx.scheduler = &scheduler_;
        logic_ctx.common_logic = proto_tcp_common_logic_;
        logic_ctx.thread_ev_base = logic_ctx_.thread_ev_base;

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_TRACE("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}

void ProtoCommonLogic::OnRecvClientMsg(const ConnGUID* conn_guid, const ::proto::MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
{
    if (proto_tcp_logic_item_vec_.size() > 0)
    {
        // 内置心跳消息处理
        if (MSG_ID_HEARTBEAT_REQ == msg_head.msg_id)
        {
            ::proto::MsgHead rsp_msg_head = msg_head;
            rsp_msg_head.msg_id = MSG_ID_HEARTBEAT_RSP;

            scheduler_.SendToClient(conn_guid, rsp_msg_head, NULL, 0);
            return;
        }

        if (0 == msg_dispatcher_.DispatchMsg(conn_guid, msg_head, msg_body, msg_body_len))
        {
            LOG_TRACE("dispatch msg ok, " << conn_guid << ", msg id: " << msg_head.msg_id);
            return;
        }
    }

    // 没有io logic或者io logic派发失败，把任务均匀分配给work线程
    if (NULL == proto_tcp_logic_args_.related_thread_groups->work_threads ||
            0 == proto_tcp_logic_args_.related_thread_groups->work_threads->GetThreadCount())
    {
        LOG_ERROR("no work threads, failed to dispatch msg, " << conn_guid << ", msg id: " << msg_head.msg_id);

        ::proto::MsgHead rsp_msg_head = msg_head;
        rsp_msg_head.msg_id = MSG_ID_NONE_HANDLER_FOUND;

        scheduler_.SendToClient(conn_guid, rsp_msg_head, NULL, 0);
        return;
    }

    if (scheduler_.SendToWorkThread(conn_guid, msg_head, msg_body, msg_body_len, -1) != 0)
    {
        LOG_ERROR("failed to send to work thread");

        ::proto::MsgHead rsp_msg_head = msg_head;
        rsp_msg_head.msg_id = MSG_ID_SCHEDULE_FAILED;

        scheduler_.SendToClient(conn_guid, rsp_msg_head, NULL, 0);
        return;
    }
}
}
}
