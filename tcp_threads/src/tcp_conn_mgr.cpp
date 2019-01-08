#include "tcp_conn_mgr.h"
#include <string.h>
#include "log_util.h"
#include "mem_util.h"
#include "buffer_event_conn.h"
#include "normal_conn.h"
#include "tcp_thread_sink.h"

namespace tcp
{
ConnMgr::ConnMgr() : conn_mgr_ctx_(), conn_hash_map_(), conn_id_seq_(), conn_id_hash_map_()
{
    thread_sink_ = NULL;
    max_online_conn_count_ = 0;
}

ConnMgr::~ConnMgr()
{
}

void ConnMgr::Release()
{
    ConnIDHashMap tmp_conn_id_hash_map = conn_id_hash_map_;

    for (ConnIDHashMap::iterator it = tmp_conn_id_hash_map.begin(); it != tmp_conn_id_hash_map.end(); ++it)
    {
        BaseConn* conn = it->second;
        conn_id_seq_.Free(conn->GetConnGUID()->conn_id);

#if defined(USE_BUFFEREVENT)
        (static_cast<BufferEventConn*>(conn))->Release();
#else
        ((NormalConn*) conn)->Release();
#endif
    }

    conn_id_hash_map_.clear();
    conn_hash_map_.clear();
    delete this;
}

int ConnMgr::Initialize(const ConnMgrCtx* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    conn_mgr_ctx_ = *(static_cast<const ConnMgrCtx*>(ctx));

    if (RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>::Initialize(conn_mgr_ctx_.timer_axis,
            conn_mgr_ctx_.inactive_conn_check_interval) != 0)
    {
        return -1;
    }

    return 0;
}

void ConnMgr::Finalize()
{
    RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>::Finalize();

    for (ConnIDHashMap::iterator it = conn_id_hash_map_.begin(); it != conn_id_hash_map_.end(); ++it)
    {
#if defined(USE_BUFFEREVENT)
        (static_cast<BufferEventConn*>(it->second))->Finalize();
#else
        ((NormalConn*) it->second)->Finalize();
#endif
    }
}

int ConnMgr::Activate()
{
    return RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>::Activate();
}

void ConnMgr::Freeze()
{
    RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>::Freeze();

    for (ConnIDHashMap::iterator it = conn_id_hash_map_.begin(); it != conn_id_hash_map_.end(); ++it)
    {
#if defined(USE_BUFFEREVENT)
        (static_cast<BufferEventConn*>(it->second))->Freeze();
#else
        ((NormalConn*) it->second)->Freeze();
#endif
    }
}

#if defined(USE_BUFFEREVENT)
BaseConn* ConnMgr::CreateBufferEventConn(int io_thread_idx, int sock_fd, struct bufferevent* buffer_event,
        const char* ip, unsigned short port)
{
    if (sock_fd < 0 || NULL == buffer_event)
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
    conn->SetClientIP(ip);
    conn->SetClientPort(port);
    conn->SetBufferEvent(buffer_event);

    int ret = -1;

    do
    {
        if (conn->Initialize(NULL) != 0)
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

        UpsertRecord(conn_id, conn, conn_mgr_ctx_.inactive_conn_life);

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
        LOG_WARN("tcp thread idx: " << conn->GetConnGUID()->io_thread_idx << ", max online tcp conn count: "
                 << max_online_conn_count_);
    }

    LOG_DEBUG("tcp thread idx: " << conn->GetConnGUID()->io_thread_idx
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
        LOG_WARN("tcp thread idx: " << conn->GetConnGUID().io_thread_idx << ", max online tcp conn count: "
                 << max_online_conn_count_);
    }

    LOG_DEBUG("tcp thread idx: " << conn->GetConnGUID().io_thread_idx
              << ", create conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
    return conn;
}
#endif

void ConnMgr::DestroyConn(int sock_fd)
{
    ConnHashMap::iterator it = conn_hash_map_.find(sock_fd);
    if (it != conn_hash_map_.end())
    {
        const ConnID conn_id = it->second->GetConnGUID()->conn_id;
        if (RecordExist(conn_id))
        {
            RemoveRecord(conn_id);
        }

        Clear(it->second);
        LOG_DEBUG("remove tcp conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
    }
    else
    {
        LOG_WARN("failed to get tcp conn by socket fd: " << sock_fd);
    }
}

BaseConn* ConnMgr::GetConn(int sock_fd) const
{
    ConnHashMap::const_iterator it = conn_hash_map_.find(sock_fd);
    if (it != conn_hash_map_.end())
    {
        return it->second;
    }

    return NULL;
}

BaseConn* ConnMgr::GetConnByID(ConnID conn_id) const
{
    ConnIDHashMap::const_iterator it = conn_id_hash_map_.find(conn_id);
    if (it != conn_id_hash_map_.end())
    {
        return it->second;
    }

    return NULL;
}

void ConnMgr::UpdateConnStatus(ConnID conn_id)
{
    ConnIDHashMap::const_iterator it = conn_id_hash_map_.find(conn_id);
    if (it != conn_id_hash_map_.end())
    {
        UpsertRecord(conn_id, it->second, conn_mgr_ctx_.inactive_conn_life);
    }
}

void ConnMgr::Clear(BaseConn* conn)
{
#if defined(USE_BUFFEREVENT)
    BufferEventConn* tcp_conn = (BufferEventConn*) conn;
    conn_hash_map_.erase(tcp_conn->GetSockFD());

    const ConnID conn_id = tcp_conn->GetConnGUID()->conn_id;
    SAFE_DESTROY(tcp_conn);

    conn_id_seq_.Free(conn_id);
    conn_id_hash_map_.erase(conn_id);
#else
    NormalConn* tcp_conn = (NormalConn*) conn;
    conn_hash_map_.erase(tcp_conn->GetSockFD());

    const ConnID conn_id = tcp_conn->GetConnID();
    SAFE_DESTROY(tcp_conn);

    conn_id_seq_.Free(conn_id);
    conn_id_hash_map_.erase(conn_id);
#endif
}

void ConnMgr::OnTimeout(const ConnID& k, BaseConn* const& v, int timeout_sec)
{
    LOG_DEBUG("ConnMgr::OnTimeout, key: " << k << ", val: " << v << ", timeout: " << timeout_sec);
    thread_sink_->OnConnTimeout(v);
    Clear(v);
}
}
