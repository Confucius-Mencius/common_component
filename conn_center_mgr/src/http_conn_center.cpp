#include "http_conn_center.h"
#include "container_util.h"
#include "log_util.h"
#include "thread_center_interface.h"

namespace http
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

    conn_center_ctx_ = *((ConnCenterCtx*) ctx);
    return 0;
}

void ConnCenter::Finalize()
{
    // http不需要自己管理inactive connections，需要重载父类的Finalize、Activate、Freeze接口
    FINALIZE_KV_CONTAINER(conn_id_hash_map_);
}

int ConnCenter::Activate()
{
    return 0;
}

void ConnCenter::Freeze()
{
    FREEZE_KV_CONTAINER(conn_id_hash_map_);
}

void ConnCenter::RemoveLogic(conn_center_mgr::BaseConn* conn)
{
    Conn* http_conn = (Conn*) conn;
    conn_hash_map_.erase(http_conn->GetEvhttpConn());

    conn_center_mgr::BaseConnCenter::RemoveConn(conn);
}

void ConnCenter::TimeoutLogic(conn_center_mgr::BaseConn* conn)
{
    // TODO http自身会检测，不会走到这里来
}

ConnInterface* ConnCenter::CreateConn(int io_thread_idx, struct evhttp_connection* evhttp_conn, bool https)
{
    if (NULL == evhttp_conn)
    {
        return NULL;
    }

    Conn* conn = Conn::Create();
    if (NULL == conn)
    {
        const int err = errno;
        LOG_ERROR("failed to create http conn, errno: " << err << ", err msg: " << strerror(errno));
        return NULL;
    }

    const ConnId conn_id = conn_id_seq_.Alloc();
    if (INVALID_CONN_ID == conn_id)
    {
        const int err = errno;
        LOG_ERROR("failed to alloc http conn id, errno: " << err << ", err msg: " << strerror(errno));
        conn->Release();
        return NULL;
    }

    // 这里先从evhttp_connection中取出对端的ip和端口
    char* peer_addr = NULL;
    ev_uint16_t peer_port = 0;
    evhttp_connection_get_peer(evhttp_conn, &peer_addr, &peer_port);

    conn->SetCreatedTime(time(NULL));
    conn->SetConnGuid(THREAD_TYPE_HTTP, io_thread_idx, conn_id);
    conn->SetEvhttpConn(evhttp_conn, https);
    conn->SetClientIp(peer_addr);
    conn->SetClientPort(peer_port);

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

        if (!conn_hash_map_.insert(ConnHashMap::value_type(evhttp_conn, conn)).second)
        {
            LOG_ERROR("failed to insert to hash map, evhttp conn: " << evhttp_conn);
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        RemoveLogic(conn);
        return NULL;
    }

    const int cur_online_client_count = (int) conn_hash_map_.size();
    if (cur_online_client_count > max_online_conn_count_)
    {
        max_online_conn_count_ = cur_online_client_count;
        LOG_INFO("http thread idx: " << conn->GetConnGuid().io_thread_idx << ", max online http conn count: "
                 << max_online_conn_count_);
    }

    LOG_TRACE("http thread idx: " << conn->GetConnGuid().io_thread_idx
              << ", create conn ok, conn id: " << conn_id
              << ", peer addr: " << peer_addr << ", port: " << peer_port);
    return conn;
}

void ConnCenter::RemoveConn(struct evhttp_connection* evhttp_conn)
{
    ConnHashMap::iterator it = conn_hash_map_.find(evhttp_conn);
    if (it != conn_hash_map_.end())
    {
        const ConnId conn_id = it->second->GetConnId();
        RemoveLogic(it->second);

        LOG_TRACE("remove http conn ok, evhttp conn: " << evhttp_conn << ", conn id: " << conn_id);
    }
    else
    {
        LOG_WARN("failed to get http conn, evhttp conn: " << evhttp_conn);
    }
}

ConnInterface* ConnCenter::GetConn(struct evhttp_connection* evhttp_conn) const
{
    ConnHashMap::const_iterator it = conn_hash_map_.find(evhttp_conn);
    if (it != conn_hash_map_.end())
    {
        return const_cast<Conn*>(it->second);
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
}
