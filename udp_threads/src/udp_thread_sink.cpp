#include "udp_thread_sink.h"
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event_struct.h>

#if defined(NDEBUG)
#include <gperftools/profiler.h>
#endif

#include "app_frame_conf_mgr_interface.h"
#include "msg_codec_center_interface.h"
#include "msg_codec_interface.h"
#include "msg_handler_interface.h"
#include "str_util.h"

// todo udp是数据报协议，是否不需要处理粘包和拆包？ 如果是这样，udp_raw就没有必要了

namespace udp
{
void ThreadSink::ReadCallback(evutil_socket_t fd, short events, void* arg)
{
    LOG_TRACE("events occured on socket, fd: " << fd << ", events: "
              << setiosflags(std::ios::showbase) << std::hex << events);
    ThreadSink* thread_sink = (ThreadSink*) arg;

    if (thread_sink->thread_->IsStopping())
    {
        LOG_WARN("in stopping status, refuse udp client data");
        return;
    }

    char* data_buf = thread_sink->GetRecvBuf();
    struct sockaddr_in client_addr;
    socklen_t peer_addr_len = sizeof(client_addr);

    ssize_t data_len = recvfrom(fd, data_buf, TOTAL_MSG_LEN_FIELD_LEN + MIN_TOTAL_MSG_LEN
                                + thread_sink->threads_ctx_->conf_mgr->GetUdpMaxMsgBodyLen(), 0,
                                (struct sockaddr*) &client_addr, &peer_addr_len);
    const int err = EVUTIL_SOCKET_ERROR();

    LOG_TRACE("recv len: " << data_len);

    if (-1 == data_len)
    {
        LOG_ERROR("failed to read, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
    }
    else if (0 == data_len)
    {
        LOG_INFO("connection closed, socket_fd: " << fd); // udp是无连接的，不会走到这里来
    }
    else if (data_len > (ssize_t) TOTAL_MSG_LEN_FIELD_LEN)
    {
        MsgId err_msg_id = MSG_ID_OK;

        if (!thread_sink->udp_msg_codec_->IsWholeMsg(err_msg_id, data_buf, data_len))
        {
            LOG_ERROR("not a whole msg, len: " << data_len << ", socket fd: " << fd);
            return;
        }

        thread_sink->OnRecvClientMsg(fd, &client_addr, data_buf + TOTAL_MSG_LEN_FIELD_LEN,
                                     data_len - TOTAL_MSG_LEN_FIELD_LEN);
    }
    else
    {
        LOG_ERROR("not a whole msg, len: " << data_len << ", socket fd: " << fd);
    }
}

ThreadSink::ThreadSink() : scheduler_(), local_logic_loader_(), logic_item_vec_()
{
    threads_ctx_ = nullptr;
    udp_thread_group_ = nullptr;
    conn_center_ = nullptr;
    udp_msg_codec_ = nullptr;
    tcp_client_center_ = nullptr;
    http_client_center_ = nullptr;
    udp_client_center_ = nullptr;
    listen_sock_fd_ = -1;
    udp_event_ = nullptr;
    recv_buf_ = nullptr;
    local_logic_ = nullptr;
    related_thread_group_ = nullptr;
}

ThreadSink::~ThreadSink()
{
}

void ThreadSink::Release()
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_RELEASE_MODULE(it->logic, it->logic_loader);
    }

    logic_item_vec_.clear();

    SAFE_RELEASE_MODULE(local_logic_, local_logic_loader_);
    delete this;
}

int ThreadSink::OnInitialize(ThreadInterface* thread)
{
    if (ThreadSinkInterface::OnInitialize(thread) != 0)
    {
        return -1;
    }

    ConnCenterCtx udp_conn_center_ctx;
    udp_conn_center_ctx.timer_axis = thread_->GetTimerAxis();

    {
        MsgCodecCtx msg_codec_ctx;
        msg_codec_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetUdpMaxMsgBodyLen();
        msg_codec_ctx.do_checksum = threads_ctx_->conf_mgr->UdpDoChecksum();

        MsgCodecInterface* msg_codec_ = threads_ctx_->msg_codec_center->CreateMsgCodec(&msg_codec_ctx);
        if (nullptr == msg_codec_)
        {
            LOG_ERROR("failed to create tcp msg codec");
            return -1;
        }

        udp_conn_center_ctx.msg_codec = msg_codec_;
    }

    udp_conn_center_ctx.inactive_conn_check_interval = {threads_ctx_->conf_mgr->GetUdpInactiveConnCheckIntervalSec(),
                                                        threads_ctx_->conf_mgr->GetUdpInactiveConnCheckIntervalUsec()
                                                       };
    udp_conn_center_ctx.inactive_conn_life = threads_ctx_->conf_mgr->GetUdpInactiveConnLife();
    udp_conn_center_ctx.max_udp_msg_len = threads_ctx_->conf_mgr->GetUdpMaxMsgBodyLen();

    conn_center_ = thread_->GetConnCenterMgr()->CreateUdpConnCenter(&udp_conn_center_ctx);
    if (nullptr == conn_center_)
    {
        return -1;
    }

    conn_center_->AddConnTimeoutSink(this);

    udp_msg_codec_ = udp_conn_center_ctx.msg_codec;

    tcp::ClientCenterCtx tcp_client_center_ctx;
    tcp_client_center_ctx.thread_ev_base = thread_->GetThreadEvBase();

    {
        MsgCodecCtx msg_codec_ctx;
        msg_codec_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetTcpMaxMsgBodyLen();
        msg_codec_ctx.do_checksum = threads_ctx_->conf_mgr->TcpDoChecksum();

        MsgCodecInterface* msg_codec_ = threads_ctx_->msg_codec_center->CreateMsgCodec(&msg_codec_ctx);
        if (nullptr == msg_codec_)
        {
            LOG_ERROR("failed to create tcp msg codec");
            return -1;
        }

        tcp_client_center_ctx.msg_codec = msg_codec_;
    }

    tcp_client_center_ctx.timer_axis = thread_->GetTimerAxis();
    tcp_client_center_ctx.trans_center = thread_->GetTransCenter();
    tcp_client_center_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetTcpMaxMsgBodyLen();
    tcp_client_center_ctx.reconnect_interval = {threads_ctx_->conf_mgr->GetPeerTcpConnIntervalSec(),
                                                threads_ctx_->conf_mgr->GetPeerTcpConnIntervalUsec()
                                               };

    tcp_client_center_ = thread_->GetClientCenterMgr()->CreateTcpClientCenter(&tcp_client_center_ctx);
    if (nullptr == tcp_client_center_)
    {
        return -1;
    }

    http::ClientCenterCtx http_client_center_ctx;
    http_client_center_ctx.thread_ev_base = thread_->GetThreadEvBase();
    http_client_center_ctx.timer_axis = thread_->GetTimerAxis();
    http_client_center_ctx.trans_center = thread_->GetTransCenter();
    http_client_center_ctx.http_conn_timeout = threads_ctx_->conf_mgr->GetPeerHttpConnTimeout();
    http_client_center_ctx.http_conn_max_retry = threads_ctx_->conf_mgr->GetPeerHttpConnMaxRetry();

    http_client_center_ = thread_->GetClientCenterMgr()->CreateHttpClientCenter(&http_client_center_ctx);
    if (nullptr == http_client_center_)
    {
        return -1;
    }

    udp::ClientCenterCtx udp_client_center_ctx;
    udp_client_center_ctx.thread_ev_base = thread_->GetThreadEvBase();
    udp_client_center_ctx.msg_codec = udp_msg_codec_;
    udp_client_center_ctx.timer_axis = thread_->GetTimerAxis();
    udp_client_center_ctx.trans_center = thread_->GetTransCenter();
    udp_client_center_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetUdpMaxMsgBodyLen();

    udp_client_center_ = thread_->GetClientCenterMgr()->CreateUdpClientCenter(&udp_client_center_ctx);
    if (nullptr == udp_client_center_)
    {
        return -1;
    }

    recv_buf_ = (char*) malloc(
                    TOTAL_MSG_LEN_FIELD_LEN + MIN_TOTAL_MSG_LEN + threads_ctx_->conf_mgr->GetUdpMaxMsgBodyLen() + 1);
    if (nullptr == recv_buf_)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc recv buf, errno: " << err << ", err msg: " << strerror(err));
        return -1;
    }

    scheduler_.SetThreadSink(this);

    if (scheduler_.Initialize(threads_ctx_) != 0)
    {
        return -1;
    }

    if (BindUdpSocket() != 0)
    {
        return -1;
    }

    if (LoadLocalLogic() != 0)
    {
        return -1;
    }

    if (LoadLogicGroup() != 0)
    {
        return -1;
    }

    return 0;
}

void ThreadSink::OnFinalize()
{
    conn_center_->RemoveConnTimeoutSink(this);

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_FINALIZE(it->logic);
    }

    SAFE_FINALIZE(local_logic_);

    if (udp_event_ != nullptr)
    {
        event_del(udp_event_);
        event_free(udp_event_);
        udp_event_ = nullptr;
    }

    if (listen_sock_fd_ != -1)
    {
        evutil_closesocket(listen_sock_fd_);
        listen_sock_fd_ = -1;
    }

    scheduler_.Finalize();

    if (recv_buf_ != nullptr)
    {
        free(recv_buf_);
        recv_buf_ = nullptr;
    }

    ThreadSinkInterface::OnFinalize();
}

int ThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(local_logic_))
    {
        return -1;
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        if (SAFE_ACTIVATE_FAILED(it->logic))
        {
            return -1;
        }
    }

    return 0;
}

void ThreadSink::OnFreeze()
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_FREEZE(it->logic);
    }

    SAFE_FREEZE(local_logic_);

    ThreadSinkInterface::OnFreeze();
}

void ThreadSink::OnThreadStartOk()
{
    ThreadSinkInterface::OnThreadStartOk();

#if defined(NDEBUG)
    if (threads_ctx_->conf_mgr->EnableCpuProfiling())
    {
        LOG_INFO("enable cpu profiling");
        ProfilerRegisterThread();
    }
#endif

    pthread_mutex_lock(threads_ctx_->frame_threads_mutex);
    ++(*threads_ctx_->frame_threads_count);
    pthread_cond_signal(threads_ctx_->frame_threads_cond);
    pthread_mutex_unlock(threads_ctx_->frame_threads_mutex);
}

void ThreadSink::OnStop()
{
    ThreadSinkInterface::OnStop();

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnStop();
    }

    if (local_logic_ != nullptr)
    {
        local_logic_->OnStop();
    }
}

void ThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnReload();
    }

    if (local_logic_ != nullptr)
    {
        local_logic_->OnReload();
    }
}

void ThreadSink::OnTask(const Task* task)
{
    ThreadSinkInterface::OnTask(task);
    const TaskCtx* task_ctx = task->GetCtx();

    switch (task_ctx->task_type)
    {
        case TASK_TYPE_UDP_SEND_TO_CLIENT:
        {
            scheduler_.SendToClient(&task_ctx->conn_guid, task_ctx->msg_head, task_ctx->msg_body,
                                    task_ctx->msg_body_len);
        }
        break;

        case TASK_TYPE_UDP_CLOSE_CONN:
        {
            scheduler_.CloseClient(&task_ctx->conn_guid);
        }
        break;

        case TASK_TYPE_NORMAL:
        {
            if (logic_item_vec_.size() > 0)
            {
                if (0 == thread_->GetMsgDispatcher()->DispatchMsg(&task_ctx->conn_guid, task_ctx->msg_head,
                        task_ctx->msg_body, task_ctx->msg_body_len))
                {
                    LOG_TRACE("dispatch msg ok, " << task_ctx->conn_guid << ", msg id: "
                              << task_ctx->msg_head.msg_id);
                    return;
                }
            }

            LOG_ERROR("failed to dispatch msg, msg id: " << task_ctx->msg_head.msg_id);
        }
        break;

        default:
        {
            LOG_ERROR("invalid task type: " << task_ctx->task_type);
        }
        break;
    }
}

bool ThreadSink::CanExit() const
{
    int can_exit = 1;

    for (LogicItemVec::const_iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        can_exit &= (it->logic->CanExit() ? 1 : 0);
    }

    if (local_logic_ != nullptr)
    {
        can_exit &= (local_logic_->CanExit() ? 1 : 0);
    }

    return (can_exit != 0);
}

void ThreadSink::OnRecvNfy(const Peer& peer, const MsgHead& msg_head, const void* msg_body, size_t msg_body_len)
{
    if (logic_item_vec_.size() > 0)
    {
        if (0 == thread_->GetMsgDispatcher()->DispatchMsg(NULL, msg_head, msg_body, msg_body_len))
        {
            LOG_TRACE("dispatch msg ok, msg id: " << msg_head.msg_id);
            return;
        }
    }

    LOG_ERROR("failed to dispatch msg, msg id: " << msg_head.msg_id);
}

void ThreadSink::OnRecvClientMsg(evutil_socket_t fd, const struct sockaddr_in* client_addr, const char* total_msg_buf,
                                 size_t total_msg_len)
{
    char client_ip[INET_ADDRSTRLEN] = "";
    unsigned short client_port = 0;

    if (evutil_inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, sizeof(client_ip)) != nullptr)
    {
        client_port = ntohs(client_addr->sin_port);
    }

    LOG_TRACE("client ip: " << client_ip << ", port: " << client_port);

    ConnInterface* udp_conn = conn_center_->GetConn(client_ip, client_port);
    if (nullptr == udp_conn)
    {
        udp_conn = conn_center_->CreateConn(thread_->GetThreadIdx(), fd, client_addr, client_ip, client_port);
        if (nullptr == udp_conn)
        {
            return;
        }
    }
    else
    {
        conn_center_->UpdateConnStatus(udp_conn->GetConnGuid().conn_id);
    }

    MsgId err_msg_id = 0;
    MsgHead msg_head;
    char* msg_body = nullptr;
    size_t msg_body_len = 0;

    if (udp_msg_codec_->DecodeMsg(err_msg_id, &msg_head, &msg_body, msg_body_len, total_msg_buf, total_msg_len) != 0)
    {
        msg_head.Reset();
        msg_head.msg_id = err_msg_id;

        udp_conn->Send(msg_head, NULL, 0, 0);
        return;
    }

    if (logic_item_vec_.size() > 0)
    {
        if (0 == thread_->GetMsgDispatcher()->DispatchMsg(&udp_conn->GetConnGuid(), msg_head, msg_body, msg_body_len))
        {
            LOG_TRACE("dispatch msg ok, " << udp_conn->GetConnGuid() << ", msg id: " << msg_head.msg_id);
            return;
        }

        app_frame::ConfMgrInterface* conf_mgr = threads_ctx_->conf_mgr;

        if (0 == conf_mgr->GetWorkThreadCount())
        {
            LOG_ERROR("failed to dispatch msg, " << udp_conn->GetConnGuid() << ", msg id: " << msg_head.msg_id);

            msg_head.Reset();
            msg_head.msg_id = MSG_ID_NONE_HANDLER_FOUND;

            udp_conn->Send(msg_head, NULL, 0, 0);
            return;
        }
    }

    // 没有io logic或者io logic派发失败，把任务均匀分配给work线程
    scheduler_.SendToWorkThread(&udp_conn->GetConnGuid(), msg_head, msg_body, msg_body_len, -1);
}

void ThreadSink::SetRelatedThreadGroup(RelatedThreadGroup* related_thread_group)
{
    related_thread_group_ = related_thread_group;

    if (related_thread_group_->global_logic != nullptr)
    {
        if (local_logic_ != nullptr)
        {
            local_logic_->SetGlobalLogic(related_thread_group_->global_logic);
        }

        for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
        {
            LogicItem& logic_item = *it;
            logic_item.logic->SetGlobalLogic(related_thread_group_->global_logic);
        }
    }

    scheduler_.SetRelatedThreadGroup(related_thread_group);
}

int ThreadSink::BindUdpSocket()
{
    app_frame::ConfMgrInterface* conf_mgr = threads_ctx_->conf_mgr;
    const std::string listen_addr_port = conf_mgr->GetUdpAddrPort();

    LOG_INFO("udp listen addr port: " << listen_addr_port);
    std::string listen_addr_port_s = listen_addr_port;

    evutil_socket_t listen_sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (listen_sock_fd_ < 0)
    {
        const int err = errno;
        LOG_ERROR("failed to create udp listen socket, errno: " << err << ", err msg: " << strerror(errno));
        return -1;
    }

    LOG_TRACE("udp listen socket fd: " << listen_sock_fd_);

    if (evutil_make_socket_nonblocking(listen_sock_fd_) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to set udp listen socket non blocking, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    if (evutil_make_listen_socket_reuseable(listen_sock_fd_) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to set udp listen socket reusable, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    if (evutil_make_listen_socket_reuseable_port(listen_sock_fd_) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to set udp listen socket reusable port, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    struct sockaddr_in local_addr;
    int local_addr_len = sizeof(local_addr);

    if (evutil_parse_sockaddr_port(listen_addr_port_s.c_str(), (struct sockaddr*) &local_addr, &local_addr_len) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to parse udp socket addr and port: " << listen_addr_port_s << ", errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    if (bind(listen_sock_fd_, (struct sockaddr*) &local_addr, local_addr_len) != 0) // listen sock fd就是与客户端收发消息的fd
    {
        const int err = errno;
        LOG_ERROR("failed to bind udp listen socket, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    udp_event_ = event_new(thread_->GetThreadEvBase(), listen_sock_fd_, EV_READ | EV_PERSIST, ThreadSink::ReadCallback,
                           this);
    if (nullptr == udp_event_)
    {
        const int err = errno;
        LOG_ERROR("failed to create udp event, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    if (event_add(udp_event_, NULL) != 0)
    {
        const int err = errno;
        LOG_ERROR("failed to add udp event, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    return 0;
}

int ThreadSink::LoadLocalLogic()
{
    if (0 == threads_ctx_->conf_mgr->GetUdpLocalLogicSo().length())
    {
        return 0;
    }

    const std::string local_logic_so_path = GetAbsolutePath(threads_ctx_->conf_mgr->GetUdpLocalLogicSo().c_str(),
                                            threads_ctx_->cur_work_dir);
    LOG_INFO("load local logic so " << local_logic_so_path << " begin");

    if (local_logic_loader_.Load(local_logic_so_path.c_str()) != 0)
    {
        LOG_ERROR("failed to load local logic so, " << local_logic_loader_.GetLastErrMsg());
        return -1;
    }

    local_logic_ = (LocalLogicInterface*) local_logic_loader_.GetModuleInterface();
    if (nullptr == local_logic_)
    {
        LOG_ERROR("failed to get local logic interface, " << local_logic_loader_.GetLastErrMsg());
        return -1;
    }

    LOG_INFO("load local logic so " << local_logic_so_path << " ...");

    LogicCtx logic_ctx;
    logic_ctx.argc = threads_ctx_->argc;
    logic_ctx.argv = threads_ctx_->argv;
    logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
    logic_ctx.cur_work_dir = threads_ctx_->cur_work_dir;
    logic_ctx.app_name = threads_ctx_->app_name;
    logic_ctx.conf_center = threads_ctx_->conf_center;
    logic_ctx.timer_axis = thread_->GetTimerAxis();
    logic_ctx.time_service = thread_->GetTimeService();
    logic_ctx.random_engine = thread_->GetRandomEngine();
    logic_ctx.conn_center = conn_center_;
    logic_ctx.msg_dispatcher = nullptr;
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.local_logic = local_logic_;
    logic_ctx.thread_ev_base = thread_->GetThreadEvBase();

    if (local_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_INFO("load local logic so " << local_logic_so_path << " end");
    return 0;
}

int ThreadSink::LoadLogicGroup()
{
    // logic group
    LogicItem logic_item;
    logic_item.logic = nullptr;

    const StrGroup logic_so_group = threads_ctx_->conf_mgr->GetUdpLogicSoGroup();

    for (StrGroup::const_iterator it = logic_so_group.begin();
            it != logic_so_group.end(); ++it)
    {
        logic_item.logic_so_path = GetAbsolutePath((*it).c_str(), threads_ctx_->cur_work_dir);
        logic_item_vec_.push_back(logic_item);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        LogicItem& logic_item = *it;
        LOG_INFO("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = (LogicInterface*) logic_item.logic_loader.GetModuleInterface();
        if (nullptr == logic_item.logic)
        {
            LOG_ERROR("failed to get logic interface, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        LOG_INFO("load logic so " << logic_item.logic_so_path << " ...");

        LogicCtx logic_ctx;
        logic_ctx.argc = threads_ctx_->argc;
        logic_ctx.argv = threads_ctx_->argv;
        logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
        logic_ctx.cur_work_dir = threads_ctx_->cur_work_dir;
        logic_ctx.app_name = threads_ctx_->app_name;
        logic_ctx.conf_center = threads_ctx_->conf_center;
        logic_ctx.timer_axis = thread_->GetTimerAxis();
        logic_ctx.time_service = thread_->GetTimeService();
        logic_ctx.random_engine = thread_->GetRandomEngine();
        logic_ctx.conn_center = conn_center_;
        logic_ctx.msg_dispatcher = thread_->GetMsgDispatcher();
        logic_ctx.scheduler = &scheduler_;
        logic_ctx.local_logic = local_logic_;
        logic_ctx.thread_ev_base = thread_->GetThreadEvBase();

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_INFO("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}

void ThreadSink::OnConnTimeout(ConnInterface* conn)
{
    conn_center_->RemoveConn(conn->GetClientIp(), conn->GetClientPort());
}
}
