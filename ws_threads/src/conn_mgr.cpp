#include "conn_mgr.h"
#include <string.h>
#include "ws_conn.h"
#include "log_util.h"
#include "mem_util.h"
#include "thread_sink.h"

namespace ws
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
        conn->Release();
    }

    conn_id_hash_map_.clear();
    conn_hash_map_.clear();
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
        it->second->Finalize();
    }
}

int ConnMgr::Activate()
{
    if (RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>::Activate() != 0)
    {
        return -1;
    }

    return 0;
}

void ConnMgr::Freeze()
{
    RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>::Freeze();

    for (ConnIDHashMap::iterator it = conn_id_hash_map_.begin(); it != conn_id_hash_map_.end(); ++it)
    {
        it->second->Freeze();
    }
}

BaseConn* ConnMgr::CreateConn(int io_thread_idx, const char* ip, unsigned short port, int sock_fd)
{
    Conn* conn = Conn::Create();

    if (NULL == conn)
    {
        const int err = errno;
        LOG_ERROR("failed to create ws conn, errno: " << err << ", err msg: " << strerror(errno));
        return NULL;
    }

    const ConnID conn_id = conn_id_seq_.Alloc();
    if (INVALID_CONN_ID == conn_id)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc ws conn id, errno: " << err << ", err msg: " << strerror(errno));
        conn->Release();
        return NULL;
    }

    const time_t now = time(NULL);

    conn->SetCreatedTime(now);
    conn->SetSockFD(sock_fd);
    conn->SetClientIP(ip);
    conn->SetClientPort(port);
    conn->SetConnGUID(io_thread_idx, conn_id);
    conn->SetThreadSink(thread_sink_);

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

        conn_hash_map_[sock_fd].conn = conn;
        conn_hash_map_[sock_fd].start_time = now;
        conn_hash_map_[sock_fd].recv_count = 0;

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
        LOG_WARN("ws thread idx: " << conn->GetConnGUID()->io_thread_idx << ", max online ws conn count: "
                 << max_online_conn_count_);
    }

    LOG_TRACE("ws thread idx: " << conn->GetConnGUID()->io_thread_idx
              << ", create conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
    return conn;
}

void ConnMgr::DestroyConn(int sock_fd)
{
    ConnHashMap::iterator it = conn_hash_map_.find(sock_fd);
    if (it != conn_hash_map_.end())
    {
        const ConnID conn_id = it->second.conn->GetConnGUID()->conn_id;

        if (RecordExist(conn_id))
        {
            RemoveRecord(conn_id);
        }

        Clear(it->second.conn);
        LOG_TRACE("destroy ws conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
    }
    else
    {
        LOG_ERROR("failed to get ws conn by socket fd: " << sock_fd);
    }
}

BaseConn* ConnMgr::GetConn(int sock_fd) const
{
    ConnHashMap::const_iterator it = conn_hash_map_.find(sock_fd);
    if (it != conn_hash_map_.end())
    {
        return it->second.conn;
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

int ConnMgr::UpdateConnStatus(ConnID conn_id)
{
    ConnIDHashMap::const_iterator it = conn_id_hash_map_.find(conn_id);
    if (it != conn_id_hash_map_.end())
    {
        const BaseConn* conn = it->second;
        const int sock_fd = conn->GetSockFD();

        // 统计收包的频率，断开疑似攻击的连接
        ++(conn_hash_map_[sock_fd].recv_count);

        const time_t now = time(NULL);
        LOG_DEBUG("socket fd: " << sock_fd << ", now: " << now
                  << ", start_time: " << conn_hash_map_[sock_fd].start_time
                  << ", recv count: " << conn_hash_map_[sock_fd].recv_count
                  << ", storm interval: " << conn_mgr_ctx_.storm_interval
                  << ", storm threshold: " << conn_mgr_ctx_.storm_threshold);

        if ((now - conn_hash_map_[sock_fd].start_time) >= conn_mgr_ctx_.storm_interval)
        {
            if (conn_hash_map_[sock_fd].recv_count >= conn_mgr_ctx_.storm_threshold)
            {
                // TODO 网络风暴测试
                LOG_WARN("net storm! conn id: " << conn_id << ", now: " << now << ", start time: "
                         << conn_hash_map_[sock_fd].start_time << ", recv count: " << conn_hash_map_[sock_fd].recv_count);
                return -1;
            }
            else
            {
                // 重新计数
                conn_hash_map_[sock_fd].start_time = now;
                conn_hash_map_[sock_fd].recv_count = 0;
            }
        }

        UpsertRecord(conn_id, it->second, conn_mgr_ctx_.inactive_conn_life);
    }

    return 0;
}

void ConnMgr::Clear(BaseConn* conn)
{
    conn_hash_map_.erase(conn->GetSockFD());

    const ConnID conn_id = conn->GetConnGUID()->conn_id;
    SAFE_DESTROY(conn);

    conn_id_seq_.Free(conn_id);
    conn_id_hash_map_.erase(conn_id);
}

void ConnMgr::OnTimeout(const ConnID& k, BaseConn* const& v, int timeout_sec)
{
    LOG_TRACE("ConnMgr::OnTimeout, key: " << k << ", val: " << v << ", timeout: " << timeout_sec);
    thread_sink_->OnClientClosed(v);
    Clear(v);
}
}
