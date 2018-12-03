#include "udp_conn_center.h"
#include "log_util.h"
#include "str_util.h"
#include "thread_center_interface.h"

namespace udp
{
ConnCenter::ConnCenter() : conn_center_ctx_(), conn_hash_map_()
{
}

ConnCenter::~ConnCenter()
{
}

void ConnCenter::Release()
{
    conn_center_mgr::BaseConnCenter::Release();
    conn_hash_map_.clear();
    delete this;
}

int ConnCenter::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    conn_center_ctx_ = *((const ConnCenterCtx*) ctx);
    inactive_conn_mgr_.SetConnCenter(this);

    if (inactive_conn_mgr_.Initialize(conn_center_ctx_.timer_axis, conn_center_ctx_.inactive_conn_check_interval) != 0)
    {
        return -1;
    }

    return 0;
}


void ConnCenter::RemoveLogic(conn_center_mgr::BaseConn* conn)
{
    Conn* udp_conn = (Conn*) conn;
    LOG_TRACE("ConnCenter::RemoveLogic, " << udp_conn->GetClientIp() << ":" << udp_conn->GetClientPort()
                  << ", socket fd: " << udp_conn->GetSockFd());

    char client_ip_port[MAX_IPV4_ADDRESS_LEN + 7] = "";

    StrPrintf(client_ip_port, sizeof(client_ip_port), "%s:%d", udp_conn->GetClientIp(), udp_conn->GetClientPort());
    LOG_TRACE(client_ip_port);
    conn_hash_map_.erase(client_ip_port);

    conn_center_mgr::BaseConnCenter::RemoveConn(conn);
}

void ConnCenter::TimeoutLogic(conn_center_mgr::BaseConn* conn)
{
    for (ConnTimeoutSinkSet::iterator it = conn_timeout_sink_set_.begin(); it != conn_timeout_sink_set_.end(); ++it)
    {
        Conn* udp_conn = (Conn*) conn;
        (*it)->OnConnTimeout((ConnInterface*) udp_conn);
    }
}

ConnInterface* ConnCenter::CreateConn(int io_thread_idx, int sock_fd, const struct sockaddr_in* client_addr,
                                      const char* ip, unsigned short port)
{
    char client_ip_port[MAX_IPV4_ADDRESS_LEN + 7] = "";
    StrPrintf(client_ip_port, sizeof(client_ip_port), "%s:%d", ip, port);

    Conn* conn = Conn::Create();
    if (NULL == conn)
    {
        const int err = errno;
        LOG_ERROR("failed to create udp conn, errno: " << err << ", err msg: " << strerror(errno));
        return NULL;
    }

    ConnId conn_id = conn_id_seq_.Alloc();
    if (INVALID_CONN_ID == conn_id)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc udp conn id, errno: " << err << ", err msg: " << strerror(errno));
        conn->Release();
        return NULL;
    }

    conn->SetCreatedTime(time(NULL));
    conn->SetConnGuid(THREAD_TYPE_UDP, io_thread_idx, conn_id);
    conn->SetSockFd(sock_fd);
    conn->SetClientIp(ip);
    conn->SetClientPort(port);
    conn->SetClientAddr(client_addr);

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

        if (!conn_hash_map_.insert(ConnHashMap::value_type(client_ip_port, conn)).second)
        {
            LOG_ERROR("failed to insert to hash map, client addr: " << client_ip_port);
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

    LOG_TRACE("udp thread idx: " << conn->GetConnGuid().io_thread_idx
                  << ", create conn ok, socket fd: " << sock_fd << ", conn id: " << conn_id
                  << ", peer addr port: " << client_ip_port);
    return conn;
}

void ConnCenter::RemoveConn(const char* ip, unsigned short port)
{
    if (NULL == ip)
    {
        return;
    }

    char client_ip_port[MAX_IPV4_ADDRESS_LEN + 7] = "";
    StrPrintf(client_ip_port, sizeof(client_ip_port), "%s:%d", ip, port);

    ConnHashMap::iterator it = conn_hash_map_.find(client_ip_port);
    if (it != conn_hash_map_.end())
    {
        const ConnId conn_id = it->second->GetConnGuid().conn_id;
        if (inactive_conn_mgr_.RecordExist(conn_id))
        {
            inactive_conn_mgr_.RemoveRecord(conn_id);
        }

        RemoveLogic(it->second);
        LOG_TRACE("remove udp conn ok, client ip port: " << client_ip_port << ", conn id: " << conn_id);
    }
    else
    {
        LOG_WARN("failed to get udp conn, client ip port: " << client_ip_port);
    }
}

ConnInterface* ConnCenter::GetConn(const char* ip, unsigned short port)
{
    if (NULL == ip)
    {
        return NULL;
    }

    char client_ip_port[MAX_IPV4_ADDRESS_LEN + 7] = "";
    StrPrintf(client_ip_port, sizeof(client_ip_port), "%s:%d", ip, port);

    ConnHashMap::iterator it = conn_hash_map_.find(client_ip_port);
    if (it != conn_hash_map_.end())
    {
        return it->second;
    }

    return NULL;
}

ConnInterface* ConnCenter::GetConn(const char* ip_port)
{
    if (NULL == ip_port)
    {
        return NULL;
    }

    ConnHashMap::iterator it = conn_hash_map_.find(ip_port);
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
