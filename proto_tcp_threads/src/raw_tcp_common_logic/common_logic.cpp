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
ProtoCommonLogic::ProtoCommonLogic()
{
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

    proto_args_ = *(static_cast<ProtoArgs*>(logic_ctx_.logic_args));

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
}

void ProtoCommonLogic::OnClientClosed(const ConnGUID* conn_guid)
{
    LOG_TRACE("conn closed, " << *conn_guid);
}

void ProtoCommonLogic::OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len)
{
    LOG_DEBUG(*conn_guid << ", data: " << data << ", len: " << len);

//    ConnRecvCtxHashTable::iterator it = conn_recv_ctx_hash_table_.find(sock_fd);
//    if (it == conn_recv_ctx_hash_table_.end())
//    {
//        return;
//    }

//    ConnRecvCtx& conn_recv_ctx = it->second;
//    MsgCodecInterface* msg_codec = tcp_msg_codec_;

//    while (true)
//    {
//        MsgHead msg_head;
//        MsgId err_msg_id = MSG_ID_OK;

//        if (conn_recv_ctx.total_msg_len_network_recved_len_ < (ssize_t) TOTAL_MSG_LEN_FIELD_LEN)
//        {
//            // 继续读头
//            ssize_t n = read(sock_fd,
//                             &(conn_recv_ctx.total_msg_len_network_[conn_recv_ctx.total_msg_len_network_recved_len_]),
//                             TOTAL_MSG_LEN_FIELD_LEN - conn_recv_ctx.total_msg_len_network_recved_len_);
//            if (0 == n)
//            {
//                LOG_TRACE("read 0, fd: " << sock_fd);
//                closed = true;
//                return;
//            }
//            else if (n < 0)
//            {
//                const int err = errno;
//                if (EINTR == err)
//                {
//                    // 被中断了，可以继续读
//                    continue;
//                }
//                else if (EAGAIN == err)
//                {
//                    // 没有数据可读了
//                    return;
//                }
//                else if (ECONNRESET == err)
//                {
//                    LOG_TRACE("conn reset by peer");
//                    closed = true;
//                    return;
//                }
//                else
//                {
//                    LOG_ERROR("read error, n: " << n << ", socked fd: " << sock_fd << ", errno: " << err
//                              << ", err msg: " << evutil_socket_error_to_string(err));
//                    return;
//                }
//            }

//            conn_recv_ctx.total_msg_len_network_recved_len_ += n;

//            if (conn_recv_ctx.total_msg_len_network_recved_len_ < (ssize_t) TOTAL_MSG_LEN_FIELD_LEN)
//            {
//                LOG_TRACE("total msg len field not recv complete, wait for next time, recv len: "
//                          << conn_recv_ctx.total_msg_len_network_recved_len_);

//                // 将该client加入一个按上一次接收到不完整消息的时间升序排列的列表,收到完整消息则从列表中移除.如果一段时间后任没有收到完整消息,则主动关闭连接
//                part_msg_mgr_.UpsertRecord(conn, sock_fd, threads_ctx_->conf_mgr->GetTcpPartMsgConnLife());

//                return;
//            }
//        }

//        if (0 == conn_recv_ctx.total_msg_recved_len_)
//        {
//            // 4个字节的头读全了，看后面的数据有多长
//            uint32_t n;
//            memcpy(&n, conn_recv_ctx.total_msg_len_network_, TOTAL_MSG_LEN_FIELD_LEN);

//            conn_recv_ctx.total_msg_len_ = ntohl(n);
//            if ((conn_recv_ctx.total_msg_len_ < (int32_t) MIN_TOTAL_MSG_LEN) ||
//                    (conn_recv_ctx.total_msg_len_ > (int32_t) max_msg_recv_len_))
//            {
//                LOG_ERROR("invalid msg len: " << conn_recv_ctx.total_msg_len_ << ", throw away all bytes in the buf");

//                // 把socket中的数据读完扔掉
//                ExhaustSocketData(sock_fd);

//                // 回复
//                msg_head.Reset();
//                msg_head.msg_id = MSG_ID_INVALID_MSG_LEN;
//                conn->Send(msg_head, NULL, 0, -1);

//                // 重置各个标记
//                conn_recv_ctx.total_msg_len_network_recved_len_ = 0;
//                conn_recv_ctx.total_msg_len_ = 0;
//                conn_recv_ctx.total_msg_recved_len_ = 0;

//                return;
//            }

//            LOG_TRACE("total msg len: " << conn_recv_ctx.total_msg_len_);
//        }

//        ssize_t n = read(sock_fd, &(conn_recv_ctx.msg_recv_buf_[conn_recv_ctx.total_msg_recved_len_]),
//                         conn_recv_ctx.total_msg_len_ - conn_recv_ctx.total_msg_recved_len_);
//        if (0 == n)
//        {
//            return;
//        }
//        else if (n < 0)
//        {
//            const int err = errno;
//            if (EINTR == err)
//            {
//                // 被中断了，可以继续读
//                continue;
//            }
//            else if (EAGAIN == err)
//            {
//                // 没有数据可读了
//                return;
//            }
//            else
//            {
//                LOG_ERROR("read error, n: " << n << ", socked fd: " << sock_fd << ", errno: " << err << ", err msg: "
//                          << evutil_socket_error_to_string(err));
//                return;
//            }
//        }

//        conn_recv_ctx.total_msg_recved_len_ += n;

//        if (conn_recv_ctx.total_msg_recved_len_ < conn_recv_ctx.total_msg_len_)
//        {
//            LOG_TRACE("not a whole msg, socket fd: " << sock_fd << ", total msg recved len: "
//                      << conn_recv_ctx.total_msg_recved_len_ << ", total msg len: "
//                      << conn_recv_ctx.total_msg_len_);

//            // 将该client加入一个按上一次接收到不完整消息的时间升序排列的列表,收到完整消息则从列表中移除.如果一段时间后任没有收到完整消息,则主动关闭连接
//            part_msg_mgr_.UpsertRecord(conn, sock_fd, threads_ctx_->conf_mgr->GetTcpPartMsgConnLife());

//            return;
//        }

//        conn_recv_ctx.msg_recv_buf_[conn_recv_ctx.total_msg_len_] = '\0';
//        part_msg_mgr_.RemoveRecord(conn, false);

//        char* msg_body = NULL;
//        size_t msg_body_len = 0;

//        msg_head.Reset();

//        if (msg_codec->DecodeMsg(err_msg_id, &msg_head, &msg_body, msg_body_len, conn_recv_ctx.msg_recv_buf_,
//                                 conn_recv_ctx.total_msg_len_) != 0)
//        {
//            msg_head.Reset();
//            msg_head.msg_id = err_msg_id;
//            conn->Send(msg_head, NULL, 0, -1);

//            // 把socket中的数据读完扔掉
//            ExhaustSocketData(sock_fd);

//            // 重置各个标记
//            conn_recv_ctx.total_msg_len_network_recved_len_ = 0;
//            conn_recv_ctx.total_msg_len_ = 0;
//            conn_recv_ctx.total_msg_recved_len_ = 0;

//            return;
//        }

//        OnRecvClientMsg(&conn->GetConnGuid(), msg_head, msg_body, msg_body_len);

//        // 重置各个标记
//        conn_recv_ctx.total_msg_len_network_recved_len_ = 0;
//        conn_recv_ctx.total_msg_len_ = 0;
//        conn_recv_ctx.total_msg_recved_len_ = 0;
//    }
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
    const std::string& proto_tcp_common_logic_so = proto_args_.app_frame_conf_mgr->GetProtoTCPCommonLogicSo();
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
        LOG_ERROR("failed to load common logic so " << proto_tcp_common_logic_so_path
                  << ", " << proto_tcp_common_logic_loader_.GetLastErrMsg());
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
    logic_ctx.scheduler = proto_args_.scheduler;
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
    LogicItem proto_tcp_logic_item;
    proto_tcp_logic_item.logic = NULL;

    const StrGroup& proto_tcp_logic_so_group = proto_args_.app_frame_conf_mgr->GetProtoTCPLogicSoGroup();

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
            LOG_ERROR("failed to load logic so " << logic_item.logic_so_path << ", "
                      << logic_item.logic_loader.GetLastErrMsg());
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
        logic_ctx.scheduler = proto_args_.scheduler;
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
}
}
