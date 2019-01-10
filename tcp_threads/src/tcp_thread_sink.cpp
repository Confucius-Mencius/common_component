#include "tcp_thread_sink.h"
#include <unistd.h>
#include <iomanip>
#include <event2/event.h>
#include <event2/event_struct.h>
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "str_util.h"
#include "task_type.h"

namespace tcp
{
const static size_t BUFFER_EVENT_MAX_SINGLE_READ = 524288; // 512k
const static size_t BUFFER_EVENT_MAX_SINGLE_WRITE = 524288;

#if defined(USE_BUFFEREVENT)
void ThreadSink::BufferEventEventCallback(struct bufferevent* buffer_event, short events, void* arg)
{
    const int err = EVUTIL_SOCKET_ERROR();
    const evutil_socket_t sock_fd = bufferevent_getfd(buffer_event);
    LOG_DEBUG("events occured on socket, fd: " << sock_fd << ", events: "
              << setiosflags(std::ios::showbase) << std::hex << events);

    ThreadSink* sink = static_cast<ThreadSink*>(arg);

    do
    {
        if (events & BEV_EVENT_ERROR)
        {
            if (err != 0)
            {
                LOG_DEBUG("error occured on socket, fd: " << sock_fd << ", errno: " << err
                          << ", err msg: " << evutil_socket_error_to_string(err));
            }

            break;
        }

        if (events & BEV_EVENT_EOF)
        {
            LOG_DEBUG("tcp conn closed, fd: " << sock_fd);
            break;
        }

        if (events & BEV_EVENT_READING)
        {
            LOG_DEBUG("reading event occurred on socket, fd: " << sock_fd);

            struct evbuffer* input_buf = bufferevent_get_input(buffer_event);
            const size_t input_buf_len = evbuffer_get_length(input_buf);
            LOG_DEBUG("input buf len: " << input_buf_len);

            if (0 == input_buf_len)
            {
                // 这个事件等价于上面的EOF事件,都表示对端关闭了
                LOG_DEBUG("tcp conn closed, socket fd: " << sock_fd);
                break;
            }
        }

        if (events & BEV_EVENT_WRITING)
        {
            LOG_DEBUG("writing event occurred on socket, fd:" << sock_fd);
        }

        if (events & BEV_EVENT_TIMEOUT)
        {
            LOG_ERROR("timeout event occurred on socket, fd: " << sock_fd);  // TODO 什么时候会出现timeout？逻辑如何处理？
        }

        return;
    } while (0);

    BaseConn* conn = sink->conn_mgr_.GetConn(sock_fd);
    if (conn != NULL)
    {
        sink->OnClientClosed(conn);
    }
}

void ThreadSink::BufferEventReadCallback(struct bufferevent* buffer_event, void* arg)
{
    struct evbuffer* input_buf = bufferevent_get_input(buffer_event);
    const evutil_socket_t sock_fd = bufferevent_getfd(buffer_event);
    LOG_DEBUG("recv data, socket fd: " << sock_fd << ", input buf len: " << evbuffer_get_length(input_buf));

    ThreadSink* sink = static_cast<ThreadSink*>(arg);

    if (sink->self_thread_->IsStopping())
    {
        LOG_WARN("in stopping status, refuse all client data");
        return;
    }

    BaseConn* conn = sink->conn_mgr_.GetConn(sock_fd);
    if (NULL == conn)
    {
        LOG_ERROR("failed to get tcp conn by socket fd: " << sock_fd);
        return;
    }

    sink->conn_mgr_.UpdateConnStatus(conn->GetConnGUID()->conn_id);
    sink->OnRecvClientData(input_buf, sock_fd, conn);
}
#else
void ThreadSink::NormalReadCallback(evutil_socket_t fd, short events, void* arg)
{
    LOG_TRACE("events occured on socket, fd: " << fd << ", events: " << setiosflags(std::ios::showbase) << std::hex
              << events);

    ThreadSink* sink = static_cast<ThreadSink*>(arg);

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

#if defined(USE_BUFFEREVENT)
ThreadSink::ThreadSink()
    : local_logic_loader_(), logic_item_vec_(), conn_mgr_(), scheduler_()
{
    threads_ctx_ = NULL;
    listen_thread_ = NULL;
    tcp_thread_group_ = NULL;
    related_thread_group_ = NULL;
    local_logic_ = NULL;
}
#else
ThreadSink::ThreadSink()
    : local_logic_loader_(), logic_item_vec_(), conn_mgr_(), scheduler_()
{
    threads_ctx_ = NULL;
    listen_thread_ = NULL;
    tcp_thread_group_ = NULL;
    related_thread_group_ = NULL;
    local_logic_ = NULL;
}
#endif

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
    conn_mgr_.Release();

    delete this;
}

int ThreadSink::OnInitialize(ThreadInterface* thread)
{
    if (ThreadSinkInterface::OnInitialize(thread) != 0)
    {
        return -1;
    }

    conn_mgr_.SetThreadSink(this);

    ConnMgrCtx conn_mgr_ctx;
    conn_mgr_ctx.timer_axis = self_thread_->GetTimerAxis();

    conn_mgr_ctx.inactive_conn_check_interval =
    {
        threads_ctx_->conf_mgr->GetTCPInactiveConnCheckIntervalSec(),
        threads_ctx_->conf_mgr->GetTCPInactiveConnCheckIntervalUsec()
    };

    conn_mgr_ctx.inactive_conn_life = threads_ctx_->conf_mgr->GetTCPInactiveConnLife();

    if (conn_mgr_.Initialize(&conn_mgr_ctx) != 0)
    {
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
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        SAFE_FINALIZE(it->logic);
    }

    SAFE_FINALIZE(local_logic_);
    scheduler_.Finalize();
    conn_mgr_.Finalize();

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

    if (conn_mgr_.Activate() != 0)
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
    conn_mgr_.Freeze();
    ThreadSinkInterface::OnFreeze();
}

void ThreadSink::OnThreadStartOK()
{
    ThreadSinkInterface::OnThreadStartOK();

    // TODO 这块怎么优化？
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

void ThreadSink::OnTask(const ThreadTask* task)
{
    ThreadSinkInterface::OnTask(task);

    switch (task->GetType())
    {
        case TASK_TYPE_TCP_CONN_CONNECTED:
        {
            NewConnCtx new_conn_ctx;
            memcpy(&new_conn_ctx, task->GetData().data(), task->GetData().size());

            if (OnClientConnected(&new_conn_ctx) != 0)
            {
                char client_ctx_buf[128] = "";
                const int n = StrPrintf(client_ctx_buf, sizeof(client_ctx_buf), "%s:%u, socket fd: %d",
                                        new_conn_ctx.client_ip, new_conn_ctx.client_port, new_conn_ctx.client_sock_fd);

                ThreadTask* task = new ThreadTask(TASK_TYPE_TCP_CONN_CLOSED, self_thread_, NULL, client_ctx_buf, n);
                if (NULL == task)
                {
                    LOG_ERROR("failed to create tcp conn closed task");
                    return;
                }

                listen_thread_->PushTask(task);
            }
        }
        break;

        case TASK_TYPE_TCP_SEND_TO_CLIENT:
        {
            scheduler_.SendToClient(task->GetConnGUID(), task->GetData().data(), task->GetData().size());
        }
        break;

        case TASK_TYPE_TCP_CLOSE_CONN:
        {
            scheduler_.CloseClient(task->GetConnGUID());
        }
        break;

        case TASK_TYPE_NORMAL:
        {
            for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
            {
                it->logic->OnTask(task->GetConnGUID(), task->GetSourceThread(),
                                  task->GetData().data(), task->GetData().size());
            }
        }
        break;

        default:
        {
            LOG_ERROR("invalid task type: " << task->GetType());
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

void ThreadSink::OnClientClosed(const BaseConn* conn)
{
    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientClosed(conn->GetConnGUID());
    }

    if (local_logic_ != NULL)
    {
        local_logic_->OnClientClosed(conn->GetConnGUID());
    }

    char client_ctx_buf[128] = "";
    const int n = StrPrintf(client_ctx_buf, sizeof(client_ctx_buf), "%s:%u, socket fd: %d",
                            conn->GetClientIP(), conn->GetClientPort(), conn->GetSockFD());

    ThreadTask* task = new ThreadTask(TASK_TYPE_TCP_CONN_CLOSED, self_thread_, NULL, client_ctx_buf, n);
    if (NULL == task)
    {
        LOG_ERROR("failed to create tcp conn closed task");
        return;
    }

    listen_thread_->PushTask(task);

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

    conn_mgr_.DestroyConn(conn->GetSockFD());
}

void ThreadSink::SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups)
{
    related_thread_group_ = related_thread_groups;

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

    scheduler_.SetRelatedThreadGroups(related_thread_groups);
}

int ThreadSink::LoadLocalLogic()
{
    const std::string tcp_local_logic_so = threads_ctx_->conf_mgr->GetTCPLocalLogicSo();
    if (0 == tcp_local_logic_so.length())
    {
        return 0;
    }

    char local_logic_so_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(local_logic_so_path, sizeof(local_logic_so_path),
                    tcp_local_logic_so.c_str(), threads_ctx_->cur_working_dir);
    LOG_DEBUG("load local logic so " << local_logic_so_path << " begin");

    if (local_logic_loader_.Load(local_logic_so_path) != 0)
    {
        LOG_ERROR("failed to load local logic so " << local_logic_so_path
                  << ", " << local_logic_loader_.GetLastErrMsg());
        return -1;
    }

    local_logic_ = static_cast<LocalLogicInterface*>(local_logic_loader_.GetModuleInterface());
    if (NULL == local_logic_)
    {
        LOG_ERROR("failed to get local logic, " << local_logic_loader_.GetLastErrMsg());
        return -1;
    }

    LogicCtx logic_ctx;
    logic_ctx.argc = threads_ctx_->argc;
    logic_ctx.argv = threads_ctx_->argv;
    logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
    logic_ctx.cur_working_dir = threads_ctx_->cur_working_dir;
    logic_ctx.app_name = threads_ctx_->app_name;
    logic_ctx.conf_center = threads_ctx_->conf_center;
    logic_ctx.timer_axis = self_thread_->GetTimerAxis();
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.local_logic = local_logic_;
    logic_ctx.thread_ev_base = self_thread_->GetThreadEvBase();

    if (local_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_INFO("load local logic so " << local_logic_so_path << " end");
    return 0;
}

int ThreadSink::LoadLogicGroup()
{
    // logic so group
    LogicItem logic_item;
    logic_item.logic = NULL;

    const StrGroup logic_so_group = threads_ctx_->conf_mgr->GetTCPLogicSoGroup();

    for (StrGroup::const_iterator it = logic_so_group.begin();
            it != logic_so_group.end(); ++it)
    {
        char logic_so_path[MAX_PATH_LEN] = "";
        GetAbsolutePath(logic_so_path, sizeof(logic_so_path), (*it).c_str(), threads_ctx_->cur_working_dir);
        logic_item.logic_so_path = logic_so_path;
        logic_item_vec_.push_back(logic_item);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        LogicItem& logic_item = *it;
        LOG_DEBUG("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so " << logic_item.logic_so_path << ", "
                      << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = static_cast<LogicInterface*>(logic_item.logic_loader.GetModuleInterface());
        if (NULL == logic_item.logic)
        {
            LOG_ERROR("failed to get logic, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        LogicCtx logic_ctx;
        logic_ctx.argc = threads_ctx_->argc;
        logic_ctx.argv = threads_ctx_->argv;
        logic_ctx.common_component_dir = threads_ctx_->common_component_dir;
        logic_ctx.cur_working_dir = threads_ctx_->cur_working_dir;
        logic_ctx.app_name = threads_ctx_->app_name;
        logic_ctx.conf_center = threads_ctx_->conf_center;
        logic_ctx.timer_axis = self_thread_->GetTimerAxis();
        logic_ctx.scheduler = &scheduler_;
        logic_ctx.local_logic = local_logic_;
        logic_ctx.thread_ev_base = self_thread_->GetThreadEvBase();

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_DEBUG("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}

int ThreadSink::OnClientConnected(const NewConnCtx* new_conn_ctx)
{
#if defined(USE_BUFFEREVENT)
    BaseConn* conn = conn_mgr_.GetConn(new_conn_ctx->client_sock_fd);
    if (conn != NULL)
    {
        LOG_WARN("tcp conn already exist, socket fd: " << new_conn_ctx->client_sock_fd << ", destroy it first");
        conn_mgr_.DestroyConn(conn->GetSockFD());
    }

    struct bufferevent* buffer_event = bufferevent_socket_new(self_thread_->GetThreadEvBase(),
                                       new_conn_ctx->client_sock_fd,
                                       BEV_OPT_CLOSE_ON_FREE);
    if (NULL == buffer_event)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to create buffer event, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        evutil_closesocket(new_conn_ctx->client_sock_fd);
        return -1;
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

    LOG_DEBUG("before set, single read size limit: " << bufferevent_get_max_single_read(buffer_event)
              << ", single write size limit: " << bufferevent_get_max_single_write(buffer_event));

    bufferevent_set_max_single_read(buffer_event, BUFFER_EVENT_MAX_SINGLE_READ); // TODO 看一下libevent源码改动还有没有必要
    bufferevent_set_max_single_write(buffer_event, BUFFER_EVENT_MAX_SINGLE_WRITE);

    LOG_DEBUG("after set, single read size limit: " << bufferevent_get_max_single_read(buffer_event)
              << ", single write size limit: " << bufferevent_get_max_single_write(buffer_event));

    bufferevent_setcb(buffer_event, ThreadSink::BufferEventReadCallback, NULL,
                      ThreadSink::BufferEventEventCallback, this);

    if (bufferevent_enable(buffer_event, EV_READ | EV_WRITE | EV_PERSIST) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to enable buffer event reading and writing, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
        bufferevent_free(buffer_event);
        return -1;
    }

    conn = conn_mgr_.CreateBufferEventConn(self_thread_->GetThreadIdx(), new_conn_ctx->client_sock_fd, buffer_event,
                                           new_conn_ctx->client_ip, new_conn_ctx->client_port);
    if (NULL == conn)
    {
        bufferevent_free(buffer_event);
        return -1;
    }

    if (local_logic_ != NULL)
    {
        local_logic_->OnClientConnected(conn->GetConnGUID());
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientConnected(conn->GetConnGUID());
    }

    return 0;
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
void ThreadSink::OnRecvClientData(struct evbuffer* input_buf, int sock_fd, BaseConn* conn)
{
    const size_t input_buf_len = evbuffer_get_length(input_buf);
    LOG_DEBUG("socket fd: " << sock_fd << ", input buf len: " << input_buf_len);

    if (0 == input_buf_len)
    {
        return;
    }

    unsigned char* data_buf = evbuffer_pullup(input_buf, input_buf_len);

    // logic处理
    if (local_logic_ != NULL)
    {
        local_logic_->OnRecvClientData(conn->GetConnGUID(), data_buf, input_buf_len);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnRecvClientData(conn->GetConnGUID(), data_buf, input_buf_len);
    }

    if (evbuffer_drain(input_buf, input_buf_len) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to drain data from input buffer, errno: " << err
                  << ", err msg: " << evutil_socket_error_to_string(err));
    }

    // TODO data_buf要不要手动释放？
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

//void ThreadSink::ExhaustSocketData(int sock_fd)
//{
//    while (true)
//    {
//        ssize_t n = read(sock_fd, msg_recv_buf_, max_msg_recv_len_);
//        if (n < 0)
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
//    }
//}
}
