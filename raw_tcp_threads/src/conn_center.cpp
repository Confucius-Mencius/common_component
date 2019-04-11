#include "conn_center.h"
#include <string.h>
#include "log_util.h"
#include "buffer_event_conn.h"
#include "io_thread_sink.h"
#include "mem_util.h"
#include "normal_conn.h"
#include "task_type.h"

namespace tcp
{
namespace raw
{
ConnCenter::ConnCenter() : conn_mgr_ctx_(), conn_hash_map_(), conn_id_seq_(), conn_id_hash_map_()
{
    thread_sink_ = nullptr;
    max_online_conn_count_ = 0;
}

ConnCenter::~ConnCenter()
{
}

void ConnCenter::Release()
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

int ConnCenter::Initialize(const ConnCenterCtx* ctx)
{
    if (nullptr == ctx)
    {
        return -1;
    }

    conn_mgr_ctx_ = *(static_cast<const ConnCenterCtx*>(ctx));

    if (RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>::Initialize(conn_mgr_ctx_.timer_axis,
            conn_mgr_ctx_.inactive_conn_check_interval) != 0)
    {
        return -1;
    }

    return 0;
}

void ConnCenter::Finalize()
{
    RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>::Finalize();

    for (ConnIDHashMap::iterator it = conn_id_hash_map_.begin(); it != conn_id_hash_map_.end(); ++it)
    {
        it->second->Finalize();
    }
}

int ConnCenter::Activate()
{
    if (RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>::Activate() != 0)
    {
        return -1;
    }

    return 0;
}

void ConnCenter::Freeze()
{
    RecordTimeoutMgr<ConnID, std::hash<ConnID>, BaseConn*>::Freeze();

    for (ConnIDHashMap::iterator it = conn_id_hash_map_.begin(); it != conn_id_hash_map_.end(); ++it)
    {
        it->second->Freeze();
    }
}

BaseConn* ConnCenter::CreateConn(IOType io_type, int io_thread_idx, const char* ip, unsigned short port, int sock_fd)
{
#if defined(USE_BUFFEREVENT)
    BufferEventConn* conn = BufferEventConn::Create();
#else
    NormalConn* conn = NormalConn::Create();
#endif

    if (nullptr == conn)
    {
        const int err = errno;
        LOG_ERROR("failed to create tcp conn, errno: " << err << ", err msg: " << strerror(errno));
        return nullptr;
    }

    conn->SetConnCenter(this);

    const ConnID conn_id = conn_id_seq_.Alloc();
    if (INVALID_CONN_ID == conn_id)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc tcp conn id, errno: " << err << ", err msg: " << strerror(errno));
        conn->Release();
        return nullptr;
    }

    const time_t now = time(nullptr);

    conn->SetCreatedTime(now);
    conn->SetConnGUID(io_type, io_thread_idx, conn_id);
    conn->SetClientIP(ip);
    conn->SetClientPort(port);
    conn->SetSockFD(sock_fd);
    conn->SetThreadSink(thread_sink_);

    int ret = -1;

    do
    {
        if (conn->Initialize(nullptr) != 0)
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
        return nullptr;
    }

    const int cur_online_conn_count = (int) conn_hash_map_.size();
    if (cur_online_conn_count > max_online_conn_count_)
    {
        max_online_conn_count_ = cur_online_conn_count;
        LOG_WARN("tcp thread idx: " << conn->GetConnGUID()->io_thread_idx << ", max online tcp conn count: "
                 << max_online_conn_count_);
    }

    LOG_TRACE("tcp thread idx: " << conn->GetConnGUID()->io_thread_idx
              << ", create conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
    return conn;
}

void ConnCenter::DestroyConn(int sock_fd)
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
        LOG_TRACE("destroy tcp conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id);
    }
    else
    {
        LOG_ERROR("failed to get tcp conn by socket fd: " << sock_fd);
    }
}

int ConnCenter::UpdateConnStatus(ConnID conn_id, bool in)
{
    ConnIDHashMap::const_iterator it = conn_id_hash_map_.find(conn_id);
    if (it != conn_id_hash_map_.cend())
    {
        if (in)
        {
            const BaseConn* conn = it->second;
            const int sock_fd = conn->GetSockFD();

            // 统计收包的频率，断开疑似攻击的连接
            ++(conn_hash_map_[sock_fd].recv_count);

            const time_t now = time(nullptr);
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
        }

        UpsertRecord(conn_id, it->second, conn_mgr_ctx_.inactive_conn_life);
    }

    return 0;
}

ConnInterface* ConnCenter::GetConnBySockFD(int sock_fd) const
{
    ConnHashMap::const_iterator it = conn_hash_map_.find(sock_fd);
    if (it != conn_hash_map_.cend())
    {
        return it->second.conn;
    }

    return nullptr;
}

ConnInterface* ConnCenter::GetConnByID(ConnID conn_id) const
{
    ConnIDHashMap::const_iterator it = conn_id_hash_map_.find(conn_id);
    if (it != conn_id_hash_map_.cend())
    {
        return it->second;
    }

    return nullptr;
}

void ConnCenter::Clear(BaseConn* conn)
{
    conn_hash_map_.erase(conn->GetSockFD());

    const ConnID conn_id = conn->GetConnGUID()->conn_id;
    SAFE_DESTROY(conn);

    conn_id_seq_.Free(conn_id);
    conn_id_hash_map_.erase(conn_id);
}

void ConnCenter::OnTimeout(const ConnID& k, BaseConn* const& v, int timeout_sec)
{
    LOG_TRACE("ConnMgr::OnTimeout, key: " << k << ", val: " << v << ", timeout: " << timeout_sec);
    thread_sink_->OnClientClosed(v, TASK_TYPE_TCP_CONN_CLOSED);
    Clear(v);
}
}
}
