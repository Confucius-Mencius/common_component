#include "tcp_conn_center.h"
#include <string.h>
#include "buffer_event_conn.h"
#include "log_util.h"
#include "mem_util.h"
#include "normal_conn.h"
#include "version.h"

namespace tcp
{
ConnCenter::ConnCenter() : conn_center_ctx_(), conn_hash_map_(), conn_id_seq_(),
    conn_id_hash_map_(), conn_inactive_sink_set_()
{
    max_online_conn_count_ = 0;
}

ConnCenter::~ConnCenter()
{
}

void ConnCenter::Clear(ConnInterface* conn)
{
#if defined(USE_BUFFEREVENT)
    BufferEventConn* tcp_conn = (BufferEventConn*) conn;
    LOG_DEBUG("ConnCenter::RemoveLogic, " << tcp_conn->GetClientIP() << ":" << tcp_conn->GetClientPort()
              << ", socket fd: " << tcp_conn->GetSockFD());

    conn_hash_map_.erase(tcp_conn->GetSockFD());

    const ConnID conn_id = tcp_conn->GetConnID();
    SAFE_DESTROY(tcp_conn);

    conn_id_seq_.Free(conn_id);
    conn_id_hash_map_.erase(conn_id);
#else
    NormalConn* tcp_conn = (NormalConn*) conn;
    LOG_DEBUG("ConnCenter::RemoveLogic, " << tcp_conn->GetClientIP() << ":" << tcp_conn->GetClientPort()
              << ", socket fd: " << tcp_conn->GetSockFD());

    conn_hash_map_.erase(tcp_conn->GetSockFD());

    const ConnID conn_id = tcp_conn->GetConnID();
    SAFE_DESTROY(tcp_conn);

    conn_id_seq_.Free(conn_id);
    conn_id_hash_map_.erase(conn_id);
#endif
}

const char* ConnCenter::GetVersion() const
{
    return TCP_CONN_CENTER_VERSION;
}

const char* ConnCenter::GetLastErrMsg() const
{
    return NULL;
}

void ConnCenter::Release()
{
    ConnIDHashMap tmp_conn_id_hash_map = conn_id_hash_map_;

    for (ConnIDHashMap::iterator it = tmp_conn_id_hash_map.begin(); it != tmp_conn_id_hash_map.end(); ++it)
    {
        ConnInterface* conn = it->second;
        conn_id_seq_.Free(conn->GetConnGUID().conn_id);

#if defined(USE_BUFFEREVENT)
        ((BufferEventConn*) conn)->Release();
#else
        ((NormalConn*) conn)->Release();
#endif
    }

    conn_id_hash_map_.clear();
    conn_hash_map_.clear();
    conn_inactive_sink_set_.clear();

    delete this;
}

int ConnCenter::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    conn_center_ctx_ = *((ConnCenterCtx*) ctx);

    if (RecordTimeoutMgr<ConnID, std::hash<ConnID>, ConnInterface*>::Initialize(
                conn_center_ctx_.timer_axis, conn_center_ctx_.inactive_conn_check_interval) != 0)
    {
        return -1;
    }

    return 0;
}

void ConnCenter::Finalize()
{
    RecordTimeoutMgr<ConnID, std::hash<ConnID>, ConnInterface*>::Finalize();

    for (ConnIDHashMap::iterator it = conn_id_hash_map_.begin(); it != conn_id_hash_map_.end(); ++it)
    {
#if defined(USE_BUFFEREVENT)
        ((BufferEventConn*) it->second)->Finalize();
#else
        ((NormalConn*) it->second)->Finalize();
#endif
    }
}

int ConnCenter::Activate()
{
    return RecordTimeoutMgr<ConnID, std::hash<ConnID>, ConnInterface*>::Activate();
}

void ConnCenter::Freeze()
{
    RecordTimeoutMgr<ConnID, std::hash<ConnID>, ConnInterface*>::Freeze();

    for (ConnIDHashMap::iterator it = conn_id_hash_map_.begin(); it != conn_id_hash_map_.end(); ++it)
    {
#if defined(USE_BUFFEREVENT)
        ((BufferEventConn*) it->second)->Freeze();
#else
        ((NormalConn*) it->second)->Freeze();
#endif
    }
}

#if defined(USE_BUFFEREVENT)
ConnInterface* ConnCenter::CreateBufferEventConn(int io_thread_idx, int sock_fd, struct bufferevent* buf_event,
        const char* ip, unsigned short port)
{
    if (sock_fd < 0 || NULL == buf_event)
    {
        return NULL;
    }

    BufferEventConn* conn = BufferEventConn::Create();
    if (NULL == conn)
    {
        const int err = errno;
        LOG_ERROR("failed to create tcp conn, errno: " << err << ", err msg: " << strerror(errno));
        return NULL;
    }

    const ConnID conn_id = conn_id_seq_.Alloc();
    if (INVALID_CONN_ID == conn_id)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc tcp conn id, errno: " << err << ", err msg: " << strerror(errno));
        conn->Release();
        return NULL;
    }

    conn->SetCreatedTime(time(NULL));
    conn->SetConnGUID(io_thread_idx, conn_id);
    conn->SetSockFD(sock_fd);
    conn->SetBufEvent(buf_event);
    conn->SetClientIP(ip);
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

        if (!conn_id_hash_map_.insert(ConnIDHashMap::value_type(conn_id, conn)).second)
        {
            LOG_ERROR("failed to insert to conn id hash map, conn id: " << conn_id);
            break;
        }

        if (!conn_hash_map_.insert(ConnHashMap::value_type(sock_fd, conn)).second)
        {
            LOG_ERROR("failed to insert to conn hash map, socket fd: " << sock_fd);
            break;
        }

        UpsertRecord(conn_id, conn, conn_center_ctx_.inactive_conn_life);

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (RecordExist(conn_id))
        {
            RemoveRecord(conn_id);
        }

        Clear(conn);
        return NULL;
    }

    const int cur_online_conn_count = (int) conn_hash_map_.size();
    if (cur_online_conn_count > max_online_conn_count_)
    {
        max_online_conn_count_ = cur_online_conn_count;
        LOG_INFO("tcp thread idx: " << conn->GetConnGUID().io_thread_idx << ", max online tcp conn count: "
                 << max_online_conn_count_);
    }

    LOG_DEBUG("tcp thread idx: " << conn->GetConnGUID().io_thread_idx
              << ", create conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
    return conn;
}
#else
ConnInterface* ConnCenter::CreateNormalConn(int io_thread_idx, int sock_fd, struct event* read_event, const char* ip,
        unsigned short port)
{
    if (sock_fd < 0 || NULL == read_event)
    {
        return NULL;
    }

    NormalConn* conn = NormalConn::Create();
    if (NULL == conn)
    {
        const int err = errno;
        LOG_ERROR("failed to create tcp conn, errno: " << err << ", err msg: " << strerror(errno));
        return NULL;
    }

    const ConnID conn_id = conn_id_seq_.Alloc();
    if (INVALID_CONN_ID == conn_id)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc tcp conn id, errno: " << err << ", err msg: " << strerror(errno));
        conn->Release();
        return NULL;
    }

    conn->SetCreatedTime(time(NULL));
    conn->SetConnGUID(io_thread_idx, conn_id);
    conn->SetSockFD(sock_fd);
    conn->SetReadEvent(read_event);
    conn->SetClientIP(ip);
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

        if (!conn_id_hash_map_.insert(ConnIDHashMap::value_type(conn_id, conn)).second)
        {
            LOG_ERROR("failed to insert to hash map, conn id: " << conn_id);
            break;
        }

        if (!conn_hash_map_.insert(ConnHashMap::value_type(sock_fd, conn)).second)
        {
            LOG_ERROR("failed to insert to hash map, socket fd: " << sock_fd);
            break;
        }

        UpsertRecord(conn_id, conn, conn_center_ctx_.inactive_conn_life);

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        if (RecordExist(conn_id))
        {
            RemoveRecord(conn_id);
        }

        Clear(conn);
        return NULL;
    }

    const int cur_online_conn_count = (int) conn_hash_map_.size();
    if (cur_online_conn_count > max_online_conn_count_)
    {
        max_online_conn_count_ = cur_online_conn_count;
        LOG_INFO("tcp thread idx: " << conn->GetConnGUID().io_thread_idx << ", max online tcp conn count: "
                 << max_online_conn_count_);
    }

    LOG_DEBUG("tcp thread idx: " << conn->GetConnGUID().io_thread_idx
              << ", create conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
    return conn;
}
#endif

void ConnCenter::DestroyConn(int sock_fd)
{
    ConnHashMap::iterator it = conn_hash_map_.find(sock_fd);
    if (it != conn_hash_map_.end())
    {
        const ConnID conn_id = it->second->GetConnGUID().conn_id;
        if (RecordExist(conn_id))
        {
            RemoveRecord(conn_id);
        }

        Clear(it->second);
        LOG_DEBUG("remove tcp conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
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

ConnInterface* ConnCenter::GetConnByID(ConnID conn_id) const
{
    ConnIDHashMap::const_iterator it = conn_id_hash_map_.find(conn_id);
    if (it != conn_id_hash_map_.end())
    {
        return it->second;
    }

    return NULL;
}

void ConnCenter::UpdateConnStatus(ConnID conn_id)
{
    ConnIDHashMap::const_iterator it = conn_id_hash_map_.find(conn_id);
    if (it != conn_id_hash_map_.end())
    {
        UpsertRecord(conn_id, it->second, conn_center_ctx_.inactive_conn_life);
    }
}

int ConnCenter::AddConnInactiveSink(ConnInactiveSinkInterface* sink)
{
    if (NULL == sink)
    {
        return -1;
    }

    ConnInactiveSinkSet::const_iterator it = conn_inactive_sink_set_.find(sink);
    if (it != conn_inactive_sink_set_.end())
    {
        LOG_ERROR("sink already exist: " << sink);
        return -1;
    }

    conn_inactive_sink_set_.insert(sink);
    return 0;
}

void ConnCenter::RemoveConnInactiveSink(ConnInactiveSinkInterface* conn_timeout_sink)
{
    ConnInactiveSinkSet::const_iterator it = conn_inactive_sink_set_.find(conn_timeout_sink);
    if (it == conn_inactive_sink_set_.end())
    {
        LOG_ERROR("sink not exist: " << conn_timeout_sink);
        return;
    }

    conn_inactive_sink_set_.erase(it);
}

void ConnCenter::OnTimeout(const ConnID& k, ConnInterface* const& v, int timeout_sec)
{
    LOG_DEBUG("ConnCenter::OnTimeout, key: " << k << ", val: " << v << ", timeout: " << timeout_sec);

    for (ConnInactiveSinkSet::iterator it = conn_inactive_sink_set_.begin(); it != conn_inactive_sink_set_.end(); ++it)
    {
        (*it)->OnConnInactive(v);
    }

    Clear(v);
}
}
