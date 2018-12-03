#include "tcp_conn_center.h"
#include "thread_center_interface.h"
#include "log_util.h"

namespace tcp
{
ConnCenter::ConnCenter() : conn_center_ctx_(), conn_hash_map_(), conn_timeout_sink_set_()
{
}

ConnCenter::~ConnCenter()
{
}

void ConnCenter::Release()
{
    conn_center_mgr::BaseConnCenter::Release();
    conn_hash_map_.clear();
    conn_timeout_sink_set_.clear();

    delete this;
}

int ConnCenter::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    conn_center_ctx_ = *((ConnCenterCtx*) ctx);
    inactive_conn_mgr_.SetConnCenter(this);

    if (inactive_conn_mgr_.Initialize(conn_center_ctx_.timer_axis, conn_center_ctx_.inactive_conn_check_interval) != 0)
    {
        return -1;
    }

    return 0;
}


void ConnCenter::RemoveLogic(conn_center_mgr::BaseConn* conn)
{
    Conn* tcp_conn = (Conn*) conn;
    LOG_TRACE("ConnCenter::RemoveLogic, " << tcp_conn->GetClientIp() << ":" << tcp_conn->GetClientPort()
              << ", socket fd: " << tcp_conn->GetSockFd());

    conn_hash_map_.erase(tcp_conn->GetSockFd());
    conn_center_mgr::BaseConnCenter::RemoveConn(conn);
}

void ConnCenter::TimeoutLogic(conn_center_mgr::BaseConn* conn)
{
    for (ConnTimeoutSinkSet::iterator it = conn_timeout_sink_set_.begin(); it != conn_timeout_sink_set_.end(); ++it)
    {
        Conn* tcp_conn = (Conn*) conn;
        (*it)->OnConnTimeout((ConnInterface*) tcp_conn);
    }
}

ConnInterface* ConnCenter::CreateBufferEventConn(int io_thread_idx, int sock_fd, struct bufferevent* buf_event,
        const char* ip, unsigned short port)
{
    if (sock_fd < 0 || NULL == buf_event)
    {
        return NULL;
    }

    Conn* conn = Conn::Create();
    if (NULL == conn)
    {
        const int err = errno;
        LOG_ERROR("failed to create tcp conn, errno: " << err << ", err msg: " << strerror(errno));
        return NULL;
    }

    const ConnId conn_id = conn_id_seq_.Alloc();
    if (INVALID_CONN_ID == conn_id)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc tcp conn id, errno: " << err << ", err msg: " << strerror(errno));
        conn->Release();
        return NULL;
    }

    conn->SetCreatedTime(time(NULL));
    conn->SetConnGuid(THREAD_TYPE_TCP, io_thread_idx, conn_id);
    conn->SetSockFd(sock_fd);
    conn->SetBufEvent(buf_event);
    conn->SetClientIp(ip);
    conn->SetClientPort(port);

    int ret = -1;

    do
    {
        if (conn->Initialize(&conn_center_ctx_) != 0)
        {
            break;
        }

        if (conn->Activate() != 0)
        {
            break;
        }

        if (!conn_id_hash_map_.insert(ConnIdHashMap::value_type(conn_id, conn)).second)
        {
            LOG_ERROR("failed to insert to hash map, conn id: " << conn_id);
            break;
        }

        if (!conn_hash_map_.insert(ConnHashMap::value_type(sock_fd, conn)).second)
        {
            LOG_ERROR("failed to insert to hash map, socket fd: " << sock_fd);
            break;
        }

        inactive_conn_mgr_.UpsertRecord(conn_id, conn, conn_center_ctx_.inactive_conn_life);

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (inactive_conn_mgr_.RecordExist(conn_id))
        {
            inactive_conn_mgr_.RemoveRecord(conn_id);
        }

        RemoveLogic(conn);
        return NULL;
    }

    const int cur_online_conn_count = (int) conn_hash_map_.size();
    if (cur_online_conn_count > max_online_conn_count_)
    {
        max_online_conn_count_ = cur_online_conn_count;
        LOG_INFO("tcp thread idx: " << conn->GetConnGuid().io_thread_idx << ", max online tcp conn count: "
                 << max_online_conn_count_);
    }

    LOG_TRACE("tcp thread idx: " << conn->GetConnGuid().io_thread_idx
              << ", create conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
    return conn;
}

ConnInterface* ConnCenter::CreateNormalConn(int io_thread_idx, int sock_fd, struct event* read_event, const char* ip,
        unsigned short port)
{
    if (sock_fd < 0 || NULL == read_event)
    {
        return NULL;
    }

    Conn* conn = Conn::Create();
    if (NULL == conn)
    {
        const int err = errno;
        LOG_ERROR("failed to create tcp conn, errno: " << err << ", err msg: " << strerror(errno));
        return NULL;
    }

    const ConnId conn_id = conn_id_seq_.Alloc();
    if (INVALID_CONN_ID == conn_id)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc tcp conn id, errno: " << err << ", err msg: " << strerror(errno));
        conn->Release();
        return NULL;
    }

    conn->SetCreatedTime(time(NULL));
    conn->SetConnGuid(THREAD_TYPE_TCP, io_thread_idx, conn_id);
    conn->SetSockFd(sock_fd);
    conn->SetReadEvent(read_event);
    conn->SetClientIp(ip);
    conn->SetClientPort(port);

    int ret = -1;

    do
    {
        if (conn->Initialize(&conn_center_ctx_) != 0)
        {
            break;
        }

        if (conn->Activate() != 0)
        {
            break;
        }

        if (!conn_id_hash_map_.insert(ConnIdHashMap::value_type(conn_id, conn)).second)
        {
            LOG_ERROR("failed to insert to hash map, conn id: " << conn_id);
            break;
        }

        if (!conn_hash_map_.insert(ConnHashMap::value_type(sock_fd, conn)).second)
        {
            LOG_ERROR("failed to insert to hash map, socket fd: " << sock_fd);
            break;
        }

        inactive_conn_mgr_.UpsertRecord(conn_id, conn, conn_center_ctx_.inactive_conn_life);

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (inactive_conn_mgr_.RecordExist(conn_id))
        {
            inactive_conn_mgr_.RemoveRecord(conn_id);
        }

        RemoveLogic(conn);
        return NULL;
    }

    const int cur_online_conn_count = (int) conn_hash_map_.size();
    if (cur_online_conn_count > max_online_conn_count_)
    {
        max_online_conn_count_ = cur_online_conn_count;
        LOG_INFO("tcp thread idx: " << conn->GetConnGuid().io_thread_idx << ", max online tcp conn count: "
                 << max_online_conn_count_);
    }

    LOG_TRACE("tcp thread idx: " << conn->GetConnGuid().io_thread_idx
              << ", create conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
    return conn;
}

void ConnCenter::RemoveConn(int sock_fd)
{
    ConnHashMap::iterator it = conn_hash_map_.find(sock_fd);
    if (it != conn_hash_map_.end())
    {
        const ConnId conn_id = it->second->GetConnGuid().conn_id;
        if (inactive_conn_mgr_.RecordExist(conn_id))
        {
            inactive_conn_mgr_.RemoveRecord(conn_id);
        }

        RemoveLogic(it->second);
        LOG_TRACE("remove tcp conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
    }
    else
    {
        LOG_WARN("failed to get tcp conn, socket fd: " << sock_fd);
    }
}

ConnInterface* ConnCenter::GetConn(int sock_fd) const
{
    ConnHashMap::const_iterator it = conn_hash_map_.find(sock_fd);
    if (it != conn_hash_map_.end())
    {
        return it->second;
    }

    return NULL;
}

ConnInterface* ConnCenter::GetConnByConnId(ConnId conn_id) const
{
    ConnIdHashMap::const_iterator it = conn_id_hash_map_.find(conn_id);
    if (it != conn_id_hash_map_.end())
    {
        return (Conn*) it->second;
    }

    return NULL;
}

void ConnCenter::UpdateConnStatus(ConnId conn_id)
{
    ConnIdHashMap::const_iterator it = conn_id_hash_map_.find(conn_id);
    if (it != conn_id_hash_map_.end())
    {
        inactive_conn_mgr_.UpsertRecord(conn_id, (Conn*) it->second, conn_center_ctx_.inactive_conn_life);
    }
}

int ConnCenter::AddConnTimeoutSink(ConnTimeoutSinkInterface* conn_timeout_sink)
{
    if (NULL == conn_timeout_sink)
    {
        LOG_ERROR("null param conn_timeout_sink");
        return -1;
    }

    ConnTimeoutSinkSet::const_iterator it = conn_timeout_sink_set_.find(conn_timeout_sink);
    if (it != conn_timeout_sink_set_.end())
    {
        LOG_ERROR("sink already exist: " << conn_timeout_sink);
        return -1;
    }

    conn_timeout_sink_set_.insert(conn_timeout_sink);
    return 0;
}

void ConnCenter::RemoveConnTimeoutSink(ConnTimeoutSinkInterface* conn_timeout_sink)
{
    ConnTimeoutSinkSet::const_iterator it = conn_timeout_sink_set_.find(conn_timeout_sink);
    if (it == conn_timeout_sink_set_.end())
    {
        LOG_ERROR("sink not exist: " << conn_timeout_sink);
        return;
    }

    conn_timeout_sink_set_.erase(it);
}
}
