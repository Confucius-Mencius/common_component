#include "tcp_thread_sink.h"
#include <unistd.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/event_struct.h>

#if defined(NDEBUG)
#include <gperftools/profiler.h>
#endif

#include "app_frame_conf_mgr_interface.h"
#include "str_util.h"

namespace tcp
{
const static size_t BUFFER_EVENT_MAX_SINGLE_READ = 524288; // 512k
const static size_t BUFFER_EVENT_MAX_SINGLE_WRITE = 524288;

#if defined(USE_BUFFEREVENT)
void ThreadSink::BufferEventEventCallback(struct bufferevent* buf_event, short events, void* arg)
{
    const int err = EVUTIL_SOCKET_ERROR();
    const evutil_socket_t sock_fd = bufferevent_getfd(buf_event);
    LOG_TRACE("events occured on socket, fd: " << sock_fd << ", events: "
              << setiosflags(std::ios::showbase) << std::hex << events);

    ThreadSink* sink = (ThreadSink*) arg;

    do
    {
        if (events & BEV_EVENT_ERROR)
        {
            if (err != 0)
            {
                LOG_TRACE("error occured on socket, fd: " << sock_fd << ", errno: " << err
                          << ", err msg: " << evutil_socket_error_to_string(err));
            }

            break;
        }

        if (events & BEV_EVENT_EOF)
        {
            LOG_TRACE("tcp conn closed, fd: " << sock_fd);
            break;
        }

        if (events & BEV_EVENT_READING)
        {
            LOG_TRACE("reading event occurred on socket, fd: " << sock_fd);

            struct evbuffer* input_buf = bufferevent_get_input(buf_event);
            const size_t input_buf_len = evbuffer_get_length(input_buf);
            LOG_TRACE("input buf len: " << input_buf_len);

            if (0 == input_buf_len)
            {
                // 这个事件等价于上面的EOF事件,都表示对端关闭了
                LOG_TRACE("tcp conn closed, socket fd: " << sock_fd);
                break;
            }
        }

        if (events & BEV_EVENT_WRITING)
        {
            LOG_TRACE("writing event occurred on socket, fd:" << sock_fd);
        }

        if (events & BEV_EVENT_TIMEOUT)
        {
            LOG_ERROR("timeout event occurred on socket, fd: " << sock_fd);  // TODO 什么时候会出现timeout？
        }

        return;
    } while (0);

    sink->CloseConn(sock_fd);
}

void ThreadSink::BufferEventReadCallback(struct bufferevent* buf_event, void* arg)
{
    struct evbuffer* input_buf = bufferevent_get_input(buf_event);
    const evutil_socket_t sock_fd = bufferevent_getfd(buf_event);
    LOG_TRACE("recv data, socket fd: " << sock_fd << ", input buf len: " << evbuffer_get_length(input_buf));

    ThreadSink* sink = (ThreadSink*) arg;

    if (sink->thread_->IsStopping())
    {
        LOG_WARN("in stopping status, refuse tcp client data");
        return;
    }

    ConnInterface* conn = sink->conn_center_->GetConn(sock_fd);
    if (NULL == conn)
    {
        LOG_ERROR("failed to get tcp conn by socket fd: " << sock_fd);
        return;
    }

    sink->conn_center_->UpdateConnStatus(conn->GetConnGuid().conn_id);

    if (sink->threads_ctx_->raw)
    {
        sink->OnClientRawData(input_buf, sock_fd, conn);
    }
    else
    {
        sink->OnClientData(input_buf, sock_fd, conn);
    }
}

#else

void ThreadSink::NormalReadCallback(evutil_socket_t fd, short events, void* arg)
{
    LOG_TRACE("events occured on socket, fd: " << fd << ", events: " << setiosflags(std::ios::showbase) << std::hex
              << events);

    ThreadSink* sink = (ThreadSink*) arg;

    do
    {
        if (events & EV_CLOSED)
        {
            LOG_INFO("tcp conn closed, fd: " << fd);
            break;
        }

        if (events & EV_READ)
        {
            if (sink->thread_->IsStopping())
            {
                LOG_WARN("in stopping status, refuse tcp client data");
                return;
            }

            ConnInterface* conn = sink->conn_center_->GetConn(fd);
            if (NULL == conn)
            {
                LOG_ERROR("failed to get tcp conn by socket fd: " << fd);
                return;
            }

            sink->conn_center_->UpdateConnStatus(conn->GetConnGuid().conn_id);
            bool closed = false;

            if (sink->threads_ctx_->raw)
            {
                sink->OnClientRawData(closed, fd, conn);
            }
            else
            {
                sink->OnClientData(closed, fd, conn);
            }

            if (closed)
            {
                // 空连接关闭，不会收到close事件，但会收到read事件，read返回0
                break;
            }
        }

        if (events & EV_TIMEOUT)
        {
            LOG_ERROR("timeout event occurred on socket, fd: " << fd);
        }

        return;
    } while (0);

    sink->CloseConn(fd);
}

#endif

ThreadSink::ThreadSink()
    : scheduler_(), part_msg_mgr_(), local_logic_loader_(), logic_item_vec_(), pre_client_blacklist_map_(),
      conn_recv_ctx_hash_table_()
{
    threads_ctx_ = NULL;
    listen_thread_ = NULL;
    tcp_thread_group_ = NULL;
    conn_center_ = NULL;
    tcp_msg_codec_ = NULL;
    tcp_client_center_ = NULL;
    http_client_center_ = NULL;
    udp_client_center_ = NULL;
    msg_recv_buf_ = NULL;
    max_msg_recv_len_ = 0;
    local_logic_ = NULL;
    related_thread_group_ = NULL;
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

    ConnCenterCtx tcp_conn_center_ctx;
    tcp_conn_center_ctx.timer_axis = thread_->GetTimerAxis();

    {
        MsgCodecCtx msg_codec_ctx;
        msg_codec_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetTcpMaxMsgBodyLen();
        msg_codec_ctx.do_checksum = threads_ctx_->conf_mgr->TcpDoChecksum();

        MsgCodecInterface* msg_codec_ = threads_ctx_->msg_codec_center->CreateMsgCodec(&msg_codec_ctx);
        if (NULL == msg_codec_)
        {
            LOG_ERROR("failed to create tcp msg codec");
            return -1;
        }

        tcp_conn_center_ctx.msg_codec = msg_codec_;
    }

    tcp_conn_center_ctx.inactive_conn_check_interval =
    {
        threads_ctx_->raw ? threads_ctx_->conf_mgr->GetRawTcpInactiveConnCheckIntervalSec()
        : threads_ctx_->conf_mgr->GetTcpInactiveConnCheckIntervalSec(),
        threads_ctx_->raw ? threads_ctx_->conf_mgr->GetRawTcpInactiveConnCheckIntervalUsec()
        : threads_ctx_->conf_mgr->GetTcpInactiveConnCheckIntervalUsec()
    };
    tcp_conn_center_ctx.inactive_conn_life = threads_ctx_->raw ? threads_ctx_->conf_mgr->GetRawTcpInactiveConnLife()
            : threads_ctx_->conf_mgr->GetTcpInactiveConnLife();
    tcp_conn_center_ctx.max_msg_body_len = threads_ctx_->raw ? 0
                                           : threads_ctx_->conf_mgr->GetTcpMaxMsgBodyLen();

    conn_center_ = thread_->GetConnCenterMgr()->CreateTcpConnCenter(&tcp_conn_center_ctx);
    if (NULL == conn_center_)
    {
        return -1;
    }

    conn_center_->AddConnTimeoutSink(this);

    tcp_msg_codec_ = tcp_conn_center_ctx.msg_codec;

    ClientCenterCtx tcp_client_center_ctx;
    tcp_client_center_ctx.thread_ev_base = thread_->GetThreadEvBase();
    tcp_client_center_ctx.msg_codec = tcp_msg_codec_;
    tcp_client_center_ctx.timer_axis = thread_->GetTimerAxis();
    tcp_client_center_ctx.trans_center = thread_->GetTransCenter();
    tcp_client_center_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetTcpMaxMsgBodyLen();
    tcp_client_center_ctx.reconnect_interval = {threads_ctx_->conf_mgr->GetPeerTcpConnIntervalSec(),
                                                threads_ctx_->conf_mgr->GetPeerTcpConnIntervalUsec()
                                               };

    tcp_client_center_ = thread_->GetClientCenterMgr()->CreateTcpClientCenter(&tcp_client_center_ctx);
    if (NULL == tcp_client_center_)
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
    if (NULL == http_client_center_)
    {
        return -1;
    }

    udp::ClientCenterCtx udp_client_center_ctx;
    udp_client_center_ctx.thread_ev_base = thread_->GetThreadEvBase();

    {
        MsgCodecCtx msg_codec_ctx;
        msg_codec_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetUdpMaxMsgBodyLen();
        msg_codec_ctx.do_checksum = threads_ctx_->conf_mgr->UdpDoChecksum();

        MsgCodecInterface* msg_codec_ = threads_ctx_->msg_codec_center->CreateMsgCodec(&msg_codec_ctx);
        if (NULL == msg_codec_)
        {
            LOG_ERROR("failed to create tcp msg codec");
            return -1;
        }

        udp_client_center_ctx.msg_codec = msg_codec_;
    }

    udp_client_center_ctx.timer_axis = thread_->GetTimerAxis();
    udp_client_center_ctx.trans_center = thread_->GetTransCenter();
    udp_client_center_ctx.max_msg_body_len = threads_ctx_->conf_mgr->GetUdpMaxMsgBodyLen();

    udp_client_center_ = thread_->GetClientCenterMgr()->CreateUdpClientCenter(&udp_client_center_ctx);
    if (NULL == udp_client_center_)
    {
        return -1;
    }

    if (threads_ctx_->raw)
    {
        max_msg_recv_len_ = BUFFER_EVENT_MAX_SINGLE_READ;
    }
    else
    {
        max_msg_recv_len_ = MIN_TOTAL_MSG_LEN + threads_ctx_->conf_mgr->GetTcpMaxMsgBodyLen();
    }

    msg_recv_buf_ = (char*) malloc(max_msg_recv_len_ + 1);
    if (NULL == msg_recv_buf_)
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
    scheduler_.Finalize();

    if (msg_recv_buf_ != NULL)
    {
        free(msg_recv_buf_);
        msg_recv_buf_ = NULL;
    }

#if !defined(USE_BUFFEREVENT)
    for (ConnRecvCtxHashTable::iterator it = conn_recv_ctx_hash_table_.begin();
            it != conn_recv_ctx_hash_table_.end(); ++it)
    {
        if (it->second.msg_recv_buf_ != NULL)
        {
            free(it->second.msg_recv_buf_);
            it->second.msg_recv_buf_ = NULL;
        }
    }

    conn_recv_ctx_hash_table_.clear();
#endif

    ThreadSinkInterface::OnFinalize();
}

int ThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    if (part_msg_mgr_.Activate() != 0)
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

    part_msg_mgr_.Freeze();
    ThreadSinkInterface::OnFreeze();
}

void ThreadSink::OnThreadStartOK()
{
    ThreadSinkInterface::OnThreadStartOK();

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

    if (local_logic_ != NULL)
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

    if (local_logic_ != NULL)
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
        case TASK_TYPE_TCP_CONN_CONNECTED:
        {
            const NewConnCtx* new_conn_ctx = (const NewConnCtx*) task_ctx->msg_body;
            OnClientConnected(new_conn_ctx);
        }
        break;

        case TASK_TYPE_TCP_SEND_TO_CLIENT:
        {
            scheduler_.SendToClient(&task_ctx->conn_guid, task_ctx->msg_head, task_ctx->msg_body,
                                    task_ctx->msg_body_len);
        }
        break;

        case TASK_TYPE_TCP_SEND_RAW_TO_CLIENT:
        {
            scheduler_.SendRawToClient(&task_ctx->conn_guid, task_ctx->msg_body, task_ctx->msg_body_len);
        }
        break;

        case TASK_TYPE_TCP_CLOSE_CONN:
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

    if (local_logic_ != NULL)
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

void ThreadSink::OnConnTimeout(ConnInterface* conn)
{
    CloseConn(conn->GetSockFd());
}

void ThreadSink::CloseConn(evutil_socket_t sock_fd)
{
    ConnInterface* conn = conn_center_->GetConn(sock_fd);
    if (conn != NULL)
    {
        OnClientClosed(conn);
    }
}

void ThreadSink::OnClientClosed(ConnInterface* conn)
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientClosed(&conn->GetConnGuid());
    }

    if (local_logic_ != NULL)
    {
        local_logic_->OnClientClosed(&conn->GetConnGuid());
    }

    TaskCtx task_ctx;
    task_ctx.task_type = TASK_TYPE_TCP_CONN_CLOSED;
    task_ctx.source_thread = thread_;

    char client_ctx_buf[128] = "";
    StrPrintf(client_ctx_buf, sizeof(client_ctx_buf), "%s:%u, socket fd: %d", conn->GetClientIp(), conn->GetClientPort(), conn->GetSockFd());
    task_ctx.msg_body = client_ctx_buf;
    task_ctx.msg_body_len = (size_t) StrLen(client_ctx_buf, sizeof(client_ctx_buf));

    Task* task = Task::Create(&task_ctx);
    if (NULL == task)
    {
        return;
    }

    if (listen_thread_->PushTask(task) != 0)
    {
        task->Release();
        return;
    }

#if !defined(USE_BUFFEREVENT)
    ConnRecvCtxHashTable::iterator it = conn_recv_ctx_hash_table_.find(conn->GetSockFd());
    if (it != conn_recv_ctx_hash_table_.end())
    {
        if (it->second.msg_recv_buf_ != NULL)
        {
            free(it->second.msg_recv_buf_);
            it->second.msg_recv_buf_ = NULL;
        }

        conn_recv_ctx_hash_table_.erase(it);
    }
#endif

    conn_center_->RemoveConn(conn->GetSockFd());
}

void ThreadSink::OnRecvClientMsg(const ConnGuid* conn_guid, const MsgHead& msg_head,
                                 const void* msg_body, size_t msg_body_len)
{
    if (logic_item_vec_.size() > 0)
    {
        // 内置心跳消息处理
        if (MSG_ID_HEARTBEAT_REQ == msg_head.msg_id)
        {
            MsgHead rsp_msg_head = msg_head;
            rsp_msg_head.msg_id = MSG_ID_HEARTBEAT_RSP;

            scheduler_.SendToClient(conn_guid, rsp_msg_head, NULL, 0);
            return;
        }

        if (0 == thread_->GetMsgDispatcher()->DispatchMsg(conn_guid, msg_head, msg_body, msg_body_len))
        {
            LOG_TRACE("dispatch msg ok, " << conn_guid << ", msg id: " << msg_head.msg_id);
            return;
        }
    }

    // 没有io logic或者io logic派发失败，把任务均匀分配给work线程
    if (NULL == related_thread_group_->work_threads ||
            0 == related_thread_group_->work_threads->GetThreadCount())
    {
        LOG_ERROR("no work threads, failed to dispatch msg, " << conn_guid << ", msg id: " << msg_head.msg_id);

        MsgHead rsp_msg_head = msg_head;
        rsp_msg_head.msg_id = MSG_ID_NONE_HANDLER_FOUND;

        scheduler_.SendToClient(conn_guid, rsp_msg_head, NULL, 0);
        return;
    }

    if (scheduler_.SendToWorkThread(conn_guid, msg_head, msg_body, msg_body_len, -1) != 0)
    {
        LOG_ERROR("failed to send to work thread");

        MsgHead rsp_msg_head = msg_head;
        rsp_msg_head.msg_id = MSG_ID_SCHEDULE_FAILED;

        scheduler_.SendToClient(conn_guid, rsp_msg_head, NULL, 0);
        return;
    }
}

void ThreadSink::SetRelatedThreadGroups(RelatedThreadGroups* related_thread_group)
{
    related_thread_group_ = related_thread_group;

    if (related_thread_group_->global_logic != NULL)
    {
        if (local_logic_ != NULL)
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

int ThreadSink::LoadLocalLogic()
{
    const std::string& tcp_local_logic_so = threads_ctx_->raw ? threads_ctx_->conf_mgr->GetRawTcpLocalLogicSo()
                                            : threads_ctx_->conf_mgr->GetTcpLocalLogicSo();
    if (0 == tcp_local_logic_so.length())
    {
        return 0;
    }

    const std::string local_logic_so_path = GetAbsolutePath(tcp_local_logic_so.c_str(),
                                            threads_ctx_->cur_working_dir);
    LOG_INFO("load local logic so " << local_logic_so_path << " begin");

    if (local_logic_loader_.Load(local_logic_so_path.c_str()) != 0)
    {
        LOG_ERROR("failed to load local logic so " << local_logic_so_path
                  << ", " << local_logic_loader_.GetLastErrMsg());
        return -1;
    }

    local_logic_ = (LocalLogicInterface*) local_logic_loader_.GetModuleInterface();
    if (NULL == local_logic_)
    {
        LOG_ERROR("failed to get local logic interface, " << local_logic_loader_.GetLastErrMsg());
        return -1;
    }

    LOG_INFO("load local logic so " << local_logic_so_path << " ...");

    LogicCtx logic_ctx;
    logic_ctx.argc = threads_ctx_->argc;
    logic_ctx.argv = threads_ctx_->argv;
    logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
    logic_ctx.cur_working_dir = threads_ctx_->cur_working_dir;
    logic_ctx.app_name = threads_ctx_->app_name;
    logic_ctx.conf_center = threads_ctx_->conf_center;
    logic_ctx.timer_axis = thread_->GetTimerAxis();
    logic_ctx.time_service = thread_->GetTimeService();
    logic_ctx.random_engine = thread_->GetRandomEngine();
    logic_ctx.conn_center = conn_center_;
    logic_ctx.msg_dispatcher = NULL;
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
    logic_item.logic = NULL;

    const StrGroup logic_so_group = threads_ctx_->raw ? threads_ctx_->conf_mgr->GetRawTcpLogicSoGroup()
                                    : threads_ctx_->conf_mgr->GetTcpLogicSoGroup();

    for (StrGroup::const_iterator it = logic_so_group.begin();
            it != logic_so_group.end(); ++it)
    {
        logic_item.logic_so_path = GetAbsolutePath((*it).c_str(), threads_ctx_->cur_working_dir);
        logic_item_vec_.push_back(logic_item);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        LogicItem& logic_item = *it;
        LOG_INFO("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so " << logic_item.logic_so_path << ", "
                      << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = (LogicInterface*) logic_item.logic_loader.GetModuleInterface();
        if (NULL == logic_item.logic)
        {
            LOG_ERROR("failed to get logic interface, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        LOG_INFO("load logic so " << logic_item.logic_so_path << " ...");

        LogicCtx logic_ctx;
        logic_ctx.argc = threads_ctx_->argc;
        logic_ctx.argv = threads_ctx_->argv;
        logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
        logic_ctx.cur_working_dir = threads_ctx_->cur_working_dir;
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

void ThreadSink::OnClientConnected(const NewConnCtx* new_conn_ctx)
{
#if defined(USE_BUFFEREVENT)
    ConnInterface* conn = conn_center_->GetConn(new_conn_ctx->client_sock_fd);
    if (conn != NULL)
    {
        LOG_WARN("tcp conn already exist, socket fd: " << new_conn_ctx->client_sock_fd << ", destroy it first");
        conn_center_->RemoveConn(conn->GetSockFd());
    }

    struct bufferevent* buf_event = bufferevent_socket_new(thread_->GetThreadEvBase(), new_conn_ctx->client_sock_fd,
                                    BEV_OPT_CLOSE_ON_FREE);
    if (NULL == buf_event)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to create buffer event, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        evutil_closesocket(new_conn_ctx->client_sock_fd);
        return;
    }

    // int send_buf_size = 0;
    // int recv_buf_size = 0;

    // GetSocketBufSize(send_buf_size, recv_buf_size, client_sock_fd);
    // LOG_TRACE("before set, send buf size: " << send_buf_size << ", recv buf size: " << recv_buf_size
    //           << ", sock_fd: " << client_sock_fd);

    // SetSocketBufSize(client_sock_fd, 4 * 1024 * 1024, 4 * 1024 * 1024); // 实际发现设置后缓冲区反而变小了

    // GetSocketBufSize(send_buf_size, recv_buf_size, client_sock_fd);
    // LOG_TRACE("after set, send buf size: " << send_buf_size << ", recv buf size: " << recv_buf_size
    //           << ", sock_fd: " << client_sock_fd);

    bufferevent_set_max_single_read(buf_event, BUFFER_EVENT_MAX_SINGLE_READ);
    bufferevent_set_max_single_write(buf_event, BUFFER_EVENT_MAX_SINGLE_WRITE);

    bufferevent_setcb(buf_event, ThreadSink::BufferEventReadCallback, NULL, ThreadSink::BufferEventEventCallback, this);

    if (bufferevent_enable(buf_event, EV_READ | EV_WRITE | EV_PERSIST) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to enable buffer event reading and writing, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        bufferevent_free(buf_event);
        return;
    }

    conn = conn_center_->CreateBufferEventConn(thread_->GetThreadIdx(), new_conn_ctx->client_sock_fd, buf_event,
            new_conn_ctx->client_ip, new_conn_ctx->client_port);
    if (NULL == conn)
    {
        bufferevent_free(buf_event);
        return;
    }

    if (local_logic_ != NULL)
    {
        local_logic_->OnClientConnected(&conn->GetConnGuid());
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientConnected(&conn->GetConnGuid());
    }
#else
    ConnInterface* conn = conn_center_->GetConn(new_conn_ctx->client_sock_fd);
    if (conn != NULL)
    {
        LOG_WARN("tcp conn already exist, socket fd: " << new_conn_ctx->client_sock_fd << ", destroy it first");
        conn_center_->RemoveConn(conn->GetSockFd());
    }

    struct event* read_event = event_new(thread_->GetThreadEvBase(), new_conn_ctx->client_sock_fd,
                                         EV_READ | EV_PERSIST | EV_CLOSED, ThreadSink::NormalReadCallback, this);
    if (NULL == read_event)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to create read event, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));

        evutil_closesocket(new_conn_ctx->client_sock_fd);
        return;
    }

    if (event_add(read_event, NULL) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to add event, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));

        event_free(read_event);
        evutil_closesocket(new_conn_ctx->client_sock_fd);

        return;
    }

    conn = conn_center_->CreateNormalConn(thread_->GetThreadIdx(), new_conn_ctx->client_sock_fd, read_event,
                                          new_conn_ctx->client_ip, new_conn_ctx->client_port);
    if (NULL == conn)
    {
        event_del(read_event);
        event_free(read_event);
        evutil_closesocket(new_conn_ctx->client_sock_fd);

        return;
    }

    if (!conn_recv_ctx_hash_table_.insert(
                ConnRecvCtxHashTable::value_type(new_conn_ctx->client_sock_fd, ConnRecvCtx())).second)
    {
        const int err = errno;
        LOG_ERROR("failed to insert to hash table, socket fd: " << new_conn_ctx->client_sock_fd
                  << ", errno: " << err << ", err msg: " << strerror(err));

        CloseConn(new_conn_ctx->client_sock_fd);
        return;
    }

    conn_recv_ctx_hash_table_[new_conn_ctx->client_sock_fd].msg_recv_buf_ = (char*) malloc(max_msg_recv_len_ + 1);
    if (NULL == conn_recv_ctx_hash_table_[new_conn_ctx->client_sock_fd].msg_recv_buf_)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc recv buf, errno: " << err << ", err msg: " << strerror(err));

        CloseConn(new_conn_ctx->client_sock_fd);
        return;
    }

    if (local_logic_ != NULL)
    {
        local_logic_->OnClientConnected(&conn->GetConnGuid());
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientConnected(&conn->GetConnGuid());
    }
#endif
}

#if defined(USE_BUFFEREVENT)

void ThreadSink::OnClientData(struct evbuffer* input_buf, const int sock_fd, ConnInterface* conn)
{
    MsgCodecInterface* msg_codec = tcp_msg_codec_;

    while (true)
    {
        MsgHead msg_head;
        MsgId err_msg_id = MSG_ID_OK;
        size_t total_msg_len = 0;

        if (!msg_codec->IsWholeMsg(err_msg_id, total_msg_len, input_buf))
        {
            if (err_msg_id != MSG_ID_NOT_A_WHOLE_MSG)
            {
                msg_head.Reset();
                msg_head.msg_id = err_msg_id;
                conn->Send(msg_head, NULL, 0, -1);

                LOG_INFO("close tcp conn, socket fd: " << sock_fd << ", err msg id: " << err_msg_id);
                CloseConn(sock_fd); // 服务器主动关闭连接

                return;
            }

            const size_t input_buf_len = evbuffer_get_length(input_buf);

            if (input_buf_len > 0)
            {
                LOG_TRACE("socket fd: " << sock_fd << ", input buf len: " << input_buf_len);

                // 将该client加入一个按上一次接收到不完整消息的时间升序排列的列表,收到完整消息则从列表中移除.如果一段时间后任没有收到完整消息,则主动关闭连接
                part_msg_mgr_.UpsertRecord(conn, sock_fd, threads_ctx_->conf_mgr->GetTcpPartMsgConnLife());
            }

            return;
        }

        part_msg_mgr_.RemoveRecord(conn);

        evbuffer_drain(input_buf, TOTAL_MSG_LEN_FIELD_LEN);

        char* total_msg_buf = msg_recv_buf_;
        if (evbuffer_remove(input_buf, total_msg_buf, total_msg_len) != (int) total_msg_len)
        {
            const int err = EVUTIL_SOCKET_ERROR();
            LOG_ERROR("failed to remove data from evbuffer, errno: " << err
                      << ", err msg: " << evutil_socket_error_to_string(err));

            msg_head.Reset();
            msg_head.msg_id = MSG_ID_REMOVE_OUT_FAILED;
            conn->Send(msg_head, NULL, 0, -1);

            return;
        }

        total_msg_buf[total_msg_len] = '\0';

        char* msg_body = NULL;
        size_t msg_body_len = 0;

        msg_head.Reset();

        if (msg_codec->DecodeMsg(err_msg_id, &msg_head, &msg_body, msg_body_len, total_msg_buf, total_msg_len) != 0)
        {
            msg_head.Reset();
            msg_head.msg_id = err_msg_id;
            conn->Send(msg_head, NULL, 0, -1);

            return;
        }

        OnRecvClientMsg(&conn->GetConnGuid(), msg_head, msg_body, msg_body_len);
    }
}

void ThreadSink::OnClientRawData(struct evbuffer* input_buf, const int sock_fd, const ConnInterface* conn)
{
    const size_t input_buf_len = evbuffer_get_length(input_buf);
    LOG_TRACE("socket fd: " << sock_fd << ", input buf len: " << input_buf_len);

    if (input_buf_len <= 0)
    {
        return;
    }

    char* data_buf = NULL;
    bool need_free = false;

    if (input_buf_len <= (size_t) BUFFER_EVENT_MAX_SINGLE_READ)
    {
        data_buf = msg_recv_buf_;
    }
    else
    {
        data_buf = new char[input_buf_len + 1];
        if (NULL == data_buf)
        {
            LOG_ERROR("failed to create data buf");
            return;
        }

        need_free = true;
    }

    if (-1 == evbuffer_remove(input_buf, data_buf, input_buf_len))
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to remove data from evbuffer, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));

        if (need_free)
        {
            delete[] data_buf;
            data_buf = NULL;
        }

        return;
    }

    data_buf[input_buf_len] = '\0';

    // logic处理
    if (local_logic_ != NULL)
    {
        local_logic_->OnRecvClientRawData(&conn->GetConnGuid(), data_buf, input_buf_len);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnRecvClientRawData(&conn->GetConnGuid(), data_buf, input_buf_len);
    }

    if (need_free)
    {
        delete[] data_buf;
        data_buf = NULL;
    }
}

#else

void ThreadSink::OnClientData(bool& closed, int sock_fd, ConnInterface* conn)
{
    ConnRecvCtxHashTable::iterator it = conn_recv_ctx_hash_table_.find(sock_fd);
    if (it == conn_recv_ctx_hash_table_.end())
    {
        return;
    }

    ConnRecvCtx& conn_recv_ctx = it->second;
    MsgCodecInterface* msg_codec = tcp_msg_codec_;

    while (true)
    {
        MsgHead msg_head;
        MsgId err_msg_id = MSG_ID_OK;

        if (conn_recv_ctx.total_msg_len_network_recved_len_ < (ssize_t) TOTAL_MSG_LEN_FIELD_LEN)
        {
            // 继续读头
            ssize_t n = read(sock_fd,
                             &(conn_recv_ctx.total_msg_len_network_[conn_recv_ctx.total_msg_len_network_recved_len_]),
                             TOTAL_MSG_LEN_FIELD_LEN - conn_recv_ctx.total_msg_len_network_recved_len_);
            if (0 == n)
            {
                LOG_TRACE("read 0, fd: " << sock_fd);
                closed = true;
                return;
            }
            else if (n < 0)
            {
                const int err = errno;
                if (EINTR == err)
                {
                    // 被中断了，可以继续读
                    continue;
                }
                else if (EAGAIN == err)
                {
                    // 没有数据可读了
                    return;
                }
                else if (ECONNRESET == err)
                {
                    LOG_TRACE("conn reset by peer");
                    closed = true;
                    return;
                }
                else
                {
                    LOG_ERROR("read error, n: " << n << ", socked fd: " << sock_fd << ", errno: " << err
                              << ", err msg: " << evutil_socket_error_to_string(err));
                    return;
                }
            }

            conn_recv_ctx.total_msg_len_network_recved_len_ += n;

            if (conn_recv_ctx.total_msg_len_network_recved_len_ < (ssize_t) TOTAL_MSG_LEN_FIELD_LEN)
            {
                LOG_TRACE("total msg len field not recv complete, wait for next time, recv len: "
                          << conn_recv_ctx.total_msg_len_network_recved_len_);

                // 将该client加入一个按上一次接收到不完整消息的时间升序排列的列表,收到完整消息则从列表中移除.如果一段时间后任没有收到完整消息,则主动关闭连接
                part_msg_mgr_.UpsertRecord(conn, sock_fd, threads_ctx_->conf_mgr->GetTcpPartMsgConnLife());

                return;
            }
        }

        if (0 == conn_recv_ctx.total_msg_recved_len_)
        {
            // 4个字节的头读全了，看后面的数据有多长
            uint32_t n;
            memcpy(&n, conn_recv_ctx.total_msg_len_network_, TOTAL_MSG_LEN_FIELD_LEN);

            conn_recv_ctx.total_msg_len_ = ntohl(n);
            if ((conn_recv_ctx.total_msg_len_ < (int32_t) MIN_TOTAL_MSG_LEN) ||
                    (conn_recv_ctx.total_msg_len_ > (int32_t) max_msg_recv_len_))
            {
                LOG_ERROR("invalid msg len: " << conn_recv_ctx.total_msg_len_ << ", throw away all bytes in the buf");

                // 把socket中的数据读完扔掉
                ExhaustSocketData(sock_fd);

                // 回复
                msg_head.Reset();
                msg_head.msg_id = MSG_ID_INVALID_MSG_LEN;
                conn->Send(msg_head, NULL, 0, -1);

                // 重置各个标记
                conn_recv_ctx.total_msg_len_network_recved_len_ = 0;
                conn_recv_ctx.total_msg_len_ = 0;
                conn_recv_ctx.total_msg_recved_len_ = 0;

                return;
            }

            LOG_TRACE("total msg len: " << conn_recv_ctx.total_msg_len_);
        }

        ssize_t n = read(sock_fd, &(conn_recv_ctx.msg_recv_buf_[conn_recv_ctx.total_msg_recved_len_]),
                         conn_recv_ctx.total_msg_len_ - conn_recv_ctx.total_msg_recved_len_);
        if (0 == n)
        {
            return;
        }
        else if (n < 0)
        {
            const int err = errno;
            if (EINTR == err)
            {
                // 被中断了，可以继续读
                continue;
            }
            else if (EAGAIN == err)
            {
                // 没有数据可读了
                return;
            }
            else
            {
                LOG_ERROR("read error, n: " << n << ", socked fd: " << sock_fd << ", errno: " << err << ", err msg: "
                          << evutil_socket_error_to_string(err));
                return;
            }
        }

        conn_recv_ctx.total_msg_recved_len_ += n;

        if (conn_recv_ctx.total_msg_recved_len_ < conn_recv_ctx.total_msg_len_)
        {
            LOG_TRACE("not a whole msg, socket fd: " << sock_fd << ", total msg recved len: "
                      << conn_recv_ctx.total_msg_recved_len_ << ", total msg len: "
                      << conn_recv_ctx.total_msg_len_);

            // 将该client加入一个按上一次接收到不完整消息的时间升序排列的列表,收到完整消息则从列表中移除.如果一段时间后任没有收到完整消息,则主动关闭连接
            part_msg_mgr_.UpsertRecord(conn, sock_fd, threads_ctx_->conf_mgr->GetTcpPartMsgConnLife());

            return;
        }

        conn_recv_ctx.msg_recv_buf_[conn_recv_ctx.total_msg_len_] = '\0';
        part_msg_mgr_.RemoveRecord(conn, false);

        char* msg_body = NULL;
        size_t msg_body_len = 0;

        msg_head.Reset();

        if (msg_codec->DecodeMsg(err_msg_id, &msg_head, &msg_body, msg_body_len, conn_recv_ctx.msg_recv_buf_,
                                 conn_recv_ctx.total_msg_len_) != 0)
        {
            msg_head.Reset();
            msg_head.msg_id = err_msg_id;
            conn->Send(msg_head, NULL, 0, -1);

            // 把socket中的数据读完扔掉
            ExhaustSocketData(sock_fd);

            // 重置各个标记
            conn_recv_ctx.total_msg_len_network_recved_len_ = 0;
            conn_recv_ctx.total_msg_len_ = 0;
            conn_recv_ctx.total_msg_recved_len_ = 0;

            return;
        }

        OnRecvClientMsg(&conn->GetConnGuid(), msg_head, msg_body, msg_body_len);

        // 重置各个标记
        conn_recv_ctx.total_msg_len_network_recved_len_ = 0;
        conn_recv_ctx.total_msg_len_ = 0;
        conn_recv_ctx.total_msg_recved_len_ = 0;
    }
}

void ThreadSink::OnClientRawData(bool& closed, int sock_fd, const ConnInterface* conn)
{
    // logic处理
    if (local_logic_ != NULL)
    {
        local_logic_->OnClientRawData(closed, &conn->GetConnGuid(), sock_fd);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientRawData(closed, &conn->GetConnGuid(), sock_fd);
    }
}

#endif

void ThreadSink::ExhaustSocketData(int sock_fd)
{
    while (true)
    {
        ssize_t n = read(sock_fd, msg_recv_buf_, max_msg_recv_len_);
        if (n < 0)
        {
            const int err = errno;
            if (EINTR == err)
            {
                // 被中断了，可以继续读
                continue;
            }
            else if (EAGAIN == err)
            {
                // 没有数据可读了
                return;
            }
            else
            {
                LOG_ERROR("read error, n: " << n << ", socked fd: " << sock_fd << ", errno: " << err << ", err msg: "
                          << evutil_socket_error_to_string(err));
                return;
            }
        }
    }
}
}
