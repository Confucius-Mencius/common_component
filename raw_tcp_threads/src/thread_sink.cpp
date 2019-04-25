#include "thread_sink.h"
#include <unistd.h>
#include "app_frame_conf_mgr_interface.h"
#include "file_util.h"
#include "str_util.h"
#include "task_type.h"

namespace tcp
{
void ThreadSink::OnAccept(struct evconnlistener* listener, evutil_socket_t sock_fd,
                          struct sockaddr* sock_addr, int sock_addr_len, void* arg)
{
    ThreadSink* thread_sink = static_cast<ThreadSink*>(arg);

    if (thread_sink->GetThread()->IsStopping())
    {
        LOG_WARN("in stopping status, refuse all new connections");
        evutil_closesocket(sock_fd);
        return;
    }

    const int tcp_conn_count_limit = thread_sink->threads_ctx_->app_frame_conf_mgr->GetTCPConnCountLimit();
    if (tcp_conn_count_limit > 0 && thread_sink->online_tcp_conn_count_ >= tcp_conn_count_limit)
    {
        LOG_ERROR("refuse new connection, online tcp conn count: " << thread_sink->online_tcp_conn_count_
                  << ", the limit is: " << tcp_conn_count_limit);
        evutil_closesocket(sock_fd);
        return;
    }

    struct sockaddr_in* client_addr = (struct sockaddr_in*) sock_addr;

    NewConnCtx new_conn_ctx;
    new_conn_ctx.client_sock_fd = sock_fd;

    if (nullptr == evutil_inet_ntop(AF_INET, &(client_addr->sin_addr),
                                    new_conn_ctx.client_ip, sizeof(new_conn_ctx.client_ip)))
    {
        LOG_ERROR("failed to get client ip, socket fd: " << sock_fd);
    }
    else
    {
        new_conn_ctx.client_port = ntohs(client_addr->sin_port);
        LOG_DEBUG("client connected, client ip: " << new_conn_ctx.client_ip << ", port: " << new_conn_ctx.client_port
                  << ", socket fd: " << sock_fd);
    }

    thread_sink->OnClientConnected(&new_conn_ctx);
}

void ThreadSink::OnListenError(evconnlistener* listener, void* arg)
{
    // 当ulimit -n较低时会报错： errno: 24, err msg: Too many open files
    const int err = EVUTIL_SOCKET_ERROR();
    evutil_socket_t sock_fd = evconnlistener_get_fd(listener);

    LOG_ERROR("err occured on socket, fd: " << sock_fd << ", errno: " << err
              << ", err msg: " << evutil_socket_error_to_string(err));
}

ThreadSink::ThreadSink() : common_logic_loader_(), logic_item_vec_(), conn_center_(),
    msg_codec_(), scheduler_(), msg_dispatcher_()
{
    threads_ctx_ = nullptr;
    listener_ = nullptr;
    online_tcp_conn_count_ = 0;
    max_online_tcp_conn_count_ = 0;
    tcp_thread_group_ = nullptr;
    related_thread_groups_ = nullptr;
    common_logic_ = nullptr;
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
    SAFE_RELEASE_MODULE(common_logic_, common_logic_loader_);
    conn_center_.Release();

    delete this;
}

int ThreadSink::OnInitialize(ThreadInterface* thread, const void* ctx)
{
    if (ThreadSinkInterface::OnInitialize(thread, ctx) != 0)
    {
        return -1;
    }

    threads_ctx_ = static_cast<const ThreadsCtx*>(ctx);

    const std::string tcp_addr_port = threads_ctx_->conf.addr + ":" + std::to_string(threads_ctx_->conf.port);
    LOG_ALWAYS("listen addr port: " << tcp_addr_port);

    struct sockaddr_in listen_sock_addr;
    int listen_sock_addr_len = sizeof(listen_sock_addr);

    // TODO 测试 ip:port, 域名:port
    if (evutil_parse_sockaddr_port(tcp_addr_port.c_str(),
                                   (struct sockaddr*) &listen_sock_addr, &listen_sock_addr_len) != 0)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to parse tcp listen socket addr port: " << tcp_addr_port
                  << ", errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    // http://www.blogjava.net/yongboy/archive/2015/02/12/422893.html
    // SO_REUSEPORT(Linux kernel 3.9)支持多个进程或者线程绑定到同一端口，提高服务器程序的性能，解决的问题：
    // 1. 允许多个套接字 bind()/listen() 同一个TCP/UDP端口
    //      每一个线程拥有自己的服务器套接字
    //      在服务器套接字上没有了锁的竞争
    // 2. 内核层面实现负载均衡
    // 3. 安全层面，监听同一个端口的套接字只能位于同一个用户下面(effective UID)
    // 4. 可用于实现服务的无缝切换（更新）
    listener_ = evconnlistener_new_bind(self_thread_->GetThreadEvBase(), ThreadSink::OnAccept, this,
                                        LEV_OPT_REUSEABLE | LEV_OPT_REUSEABLE_PORT | LEV_OPT_CLOSE_ON_EXEC
                                        | LEV_OPT_CLOSE_ON_FREE | LEV_OPT_DEFERRED_ACCEPT, -1,
                                        (struct sockaddr*) &listen_sock_addr, listen_sock_addr_len);
    if (nullptr == listener_)
    {
        const int err = EVUTIL_SOCKET_ERROR();
        LOG_ERROR("failed to create listener, errno: " << err << ", err msg: " << evutil_socket_error_to_string(err));
        return -1;
    }

    evconnlistener_set_error_cb(listener_, ThreadSink::OnListenError);

    ::proto::MsgCodecCtx msg_codec_ctx;
    msg_codec_ctx.max_msg_body_len = threads_ctx_->app_frame_conf_mgr->GetProtoMaxMsgBodyLen();
    msg_codec_ctx.do_checksum = threads_ctx_->app_frame_conf_mgr->ProtoDoChecksum();
    msg_codec_.SetCtx(&msg_codec_ctx);

    conn_center_.SetThreadSink(this);

    ConnCenterCtx conn_center_ctx;
    conn_center_ctx.timer_axis = self_thread_->GetTimerAxis();
    conn_center_ctx.use_bufferevent = threads_ctx_->conf.use_bufferevent;

    conn_center_ctx.inactive_conn_check_interval =
    {
        threads_ctx_->app_frame_conf_mgr->GetTCPInactiveConnCheckIntervalSec(),
        threads_ctx_->app_frame_conf_mgr->GetTCPInactiveConnCheckIntervalUsec()
    };

    conn_center_ctx.inactive_conn_life = threads_ctx_->app_frame_conf_mgr->GetTCPInactiveConnLife();
    conn_center_ctx.storm_interval = threads_ctx_->app_frame_conf_mgr->GetTCPStormInterval();
    conn_center_ctx.storm_threshold = threads_ctx_->app_frame_conf_mgr->GetTCPStormThreshold();

    if (conn_center_.Initialize(&conn_center_ctx) != 0)
    {
        return -1;
    }

    scheduler_.SetThreadSink(this);
    scheduler_.SetMsgCodec(&msg_codec_);

    if (scheduler_.Initialize(threads_ctx_) != 0)
    {
        return -1;
    }

    if (LoadCommonLogic() != 0)
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

    SAFE_FINALIZE(common_logic_);
    scheduler_.Finalize();
    conn_center_.Finalize();

    if (listener_ != nullptr)
    {
        evconnlistener_free(listener_);
        listener_ = nullptr;
    }

    ThreadSinkInterface::OnFinalize();
}

int ThreadSink::OnActivate()
{
    if (ThreadSinkInterface::OnActivate() != 0)
    {
        return -1;
    }

    if (conn_center_.Activate() != 0)
    {
        return -1;
    }

    if (SAFE_ACTIVATE_FAILED(common_logic_))
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

    SAFE_FREEZE(common_logic_);
    conn_center_.Freeze();
    ThreadSinkInterface::OnFreeze();
}

void ThreadSink::OnThreadStartOK()
{
    ThreadSinkInterface::OnThreadStartOK();

    pthread_mutex_lock(threads_ctx_->app_frame_threads_sync_mutex);
    ++(*threads_ctx_->app_frame_threads_count);
    pthread_cond_signal(threads_ctx_->app_frame_threads_sync_cond);
    pthread_mutex_unlock(threads_ctx_->app_frame_threads_sync_mutex);
}

void ThreadSink::OnStop()
{
    ThreadSinkInterface::OnStop();

    if (common_logic_ != nullptr)
    {
        common_logic_->OnStop();
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnStop();
    }
}

void ThreadSink::OnReload()
{
    ThreadSinkInterface::OnReload();

    if (common_logic_ != nullptr)
    {
        common_logic_->OnReload();
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        it->logic->OnReload();
    }
}

void ThreadSink::OnTask(const ThreadTask* task)
{
    ThreadSinkInterface::OnTask(task);

    switch (task->GetType())
    {
        case TASK_TYPE_SEND_TO_CLIENT:
        {
            scheduler_.SendToClient(task->GetConnGUID(), task->GetData().data(), task->GetData().size());
        }
        break;

        case TASK_TYPE_CLOSE_CONN:
        {
            scheduler_.CloseClient(task->GetConnGUID());
        }
        break;

        case TASK_TYPE_NORMAL:
        {
            const char* data = task->GetData().data();
            size_t len = task->GetData().size();

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

            const ConnGUID* conn_guid = task->GetConnGUID();
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
                if (common_logic_ != nullptr)
                {
                    common_logic_->OnTask(task->GetConnGUID(), task->GetSourceThread(),
                                          task->GetData().data(), task->GetData().size());
                }

                for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
                {
                    it->logic->OnTask(task->GetConnGUID(), task->GetSourceThread(),
                                      task->GetData().data(), task->GetData().size());
                }
            }
        }
        break;

        default:
        {
            LOG_ERROR("invalid task type: " << task->GetType());
            return;
        }
        break;
    }
}

bool ThreadSink::CanExit() const
{
    int can_exit = 1;

    if (common_logic_ != nullptr)
    {
        can_exit &= (common_logic_->CanExit() ? 1 : 0);
    }

    for (LogicItemVec::const_iterator it = logic_item_vec_.cbegin(); it != logic_item_vec_.cend(); ++it)
    {
        can_exit &= (it->logic->CanExit() ? 1 : 0);
    }

    return (can_exit != 0);
}

void ThreadSink::OnClientClosed(const BaseConn* conn)
{
    LOG_TRACE("ThreadSink::OnClientClosed");

    if (common_logic_ != nullptr)
    {
        common_logic_->OnClientClosed(conn->GetConnGUID());
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientClosed(conn->GetConnGUID());
    }

    conn_center_.DestroyConn(conn->GetSockFD());
    --online_tcp_conn_count_;
}

void ThreadSink::OnRecvClientData(const ConnGUID* conn_guid, const void* data, size_t len)
{
    if (common_logic_ != nullptr)
    {
        common_logic_->OnRecvClientData(conn_guid, data, len);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnRecvClientData(conn_guid, data, len);
    }
}

void ThreadSink::SetRelatedThreadGroups(RelatedThreadGroups* related_thread_groups)
{
    related_thread_groups_ = related_thread_groups;

    if (related_thread_groups_->global_logic != nullptr)
    {
        if (common_logic_ != nullptr)
        {
            common_logic_->SetGlobalLogic(related_thread_groups_->global_logic);
        }

        for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
        {
            LogicItem& logic_item = *it;
            logic_item.logic->SetGlobalLogic(related_thread_groups_->global_logic);
        }
    }

    scheduler_.SetRelatedThreadGroups(related_thread_groups);
}

int ThreadSink::LoadCommonLogic()
{
    const std::string& common_logic_so = threads_ctx_->conf.common_logic_so;
    if (0 == common_logic_so.length())
    {
        return 0;
    }

    char common_logic_so_path[MAX_PATH_LEN] = "";
    GetAbsolutePath(common_logic_so_path, sizeof(common_logic_so_path),
                    common_logic_so.c_str(), threads_ctx_->cur_working_dir);
    LOG_ALWAYS("load common logic so " << common_logic_so_path << " begin");

    if (common_logic_loader_.Load(common_logic_so_path) != 0)
    {
        LOG_ERROR("failed to load common logic so, " << common_logic_loader_.GetLastErrMsg());
        return -1;
    }

    common_logic_ = static_cast<CommonLogicInterface*>(common_logic_loader_.GetModuleInterface());
    if (nullptr == common_logic_)
    {
        LOG_ERROR("failed to get common logic, " << common_logic_loader_.GetLastErrMsg());
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
    logic_ctx.conn_center = &conn_center_;
    logic_ctx.scheduler = &scheduler_;
    logic_ctx.msg_dispatcher = &msg_dispatcher_;
    logic_ctx.common_logic = common_logic_;
    logic_ctx.thread_ev_base = self_thread_->GetThreadEvBase();
    logic_ctx.thread_idx = self_thread_->GetThreadIdx();
    logic_ctx.logic_args = threads_ctx_->logic_args;

    if (common_logic_->Initialize(&logic_ctx) != 0)
    {
        return -1;
    }

    LOG_ALWAYS("load common logic so " << common_logic_so_path << " end");
    return 0;
}

int ThreadSink::LoadLogicGroup()
{
    // logic so group
    const StrGroup& logic_so_group = threads_ctx_->conf.logic_so_group;

    for (StrGroup::const_iterator it = logic_so_group.cbegin(); it != logic_so_group.cend(); ++it)
    {
        char logic_so_path[MAX_PATH_LEN] = "";
        GetAbsolutePath(logic_so_path, sizeof(logic_so_path), (*it).c_str(), threads_ctx_->cur_working_dir);

        LogicItem logic_item;
        logic_item.logic_so_path = logic_so_path;
        logic_item.logic = nullptr;

        logic_item_vec_.push_back(logic_item);
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        LogicItem& logic_item = *it;
        LOG_ALWAYS("load logic so " << logic_item.logic_so_path << " begin");

        if (logic_item.logic_loader.Load(logic_item.logic_so_path.c_str()) != 0)
        {
            LOG_ERROR("failed to load logic so, " << logic_item.logic_loader.GetLastErrMsg());
            return -1;
        }

        logic_item.logic = static_cast<LogicInterface*>(logic_item.logic_loader.GetModuleInterface());
        if (nullptr == logic_item.logic)
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
        logic_ctx.conn_center = &conn_center_;
        logic_ctx.scheduler = &scheduler_;
        logic_ctx.msg_dispatcher = &msg_dispatcher_;
        logic_ctx.common_logic = common_logic_;
        logic_ctx.thread_ev_base = self_thread_->GetThreadEvBase();
        logic_ctx.thread_idx = self_thread_->GetThreadIdx();
        logic_ctx.logic_args = threads_ctx_->logic_args;

        if (logic_item.logic->Initialize(&logic_ctx) != 0)
        {
            return -1;
        }

        LOG_ALWAYS("load logic so " << logic_item.logic_so_path << " end");
    }

    return 0;
}

int ThreadSink::OnClientConnected(const NewConnCtx* new_conn_ctx)
{
    BaseConn* conn = static_cast<BaseConn*>(conn_center_.GetConnBySockFD(new_conn_ctx->client_sock_fd));
    if (conn != nullptr)
    {
        LOG_WARN("tcp conn already exist, socket fd: " << new_conn_ctx->client_sock_fd << ", destroy it first");
        conn_center_.DestroyConn(conn->GetSockFD());
        --online_tcp_conn_count_;
    }

    conn = conn_center_.CreateConn(threads_ctx_->conf.io_type, self_thread_->GetThreadIdx(), new_conn_ctx->client_ip,
                                   new_conn_ctx->client_port, new_conn_ctx->client_sock_fd);
    if (nullptr == conn)
    {
        return -1;
    }

    ++online_tcp_conn_count_;

    if (online_tcp_conn_count_ > max_online_tcp_conn_count_)
    {
        max_online_tcp_conn_count_ = online_tcp_conn_count_;
        LOG_WARN("max online tcp conn count: " << max_online_tcp_conn_count_);
    }

    if (common_logic_ != nullptr)
    {
        common_logic_->OnClientConnected(conn->GetConnGUID());
    }

    for (LogicItemVec::iterator it = logic_item_vec_.begin(); it != logic_item_vec_.end(); ++it)
    {
        (*it).logic->OnClientConnected(conn->GetConnGUID());
    }

    return 0;
}
}
