#include "conn_center_mgr.h"
#include "container_util.h"
#include "log_util.h"
#include "version.h"

namespace conn_center_mgr
{
ConnCenterMgr::ConnCenterMgr() : tcp_conn_center_set_(), http_conn_center_set_(), udp_conn_center_set_()
{
}

ConnCenterMgr::~ConnCenterMgr()
{
}

const char* ConnCenterMgr::GetVersion() const
{
    return CONN_CENTER_MGR_CONN_CENTER_MGR_VERSION;
}

const char* ConnCenterMgr::GetLastErrMsg() const
{
    return nullptr;
}

void ConnCenterMgr::Release()
{
    RELEASE_CONTAINER(tcp_conn_center_set_);
    RELEASE_CONTAINER(http_conn_center_set_);
    RELEASE_CONTAINER(udp_conn_center_set_);

    delete this;
}

int ConnCenterMgr::Initialize(const void* ctx)
{
    return 0;
}

void ConnCenterMgr::Finalize()
{
    FINALIZE_CONTAINER(tcp_conn_center_set_);
    FINALIZE_CONTAINER(http_conn_center_set_);
    FINALIZE_CONTAINER(udp_conn_center_set_);
}

int ConnCenterMgr::Activate()
{
    return 0;
}

void ConnCenterMgr::Freeze()
{
    FREEZE_CONTAINER(tcp_conn_center_set_);
    FREEZE_CONTAINER(http_conn_center_set_);
    FREEZE_CONTAINER(udp_conn_center_set_);
}

tcp::ConnCenterInterface* ConnCenterMgr::CreateTcpConnCenter(const tcp::ConnCenterCtx* ctx)
{
    tcp::ConnCenter* conn_center = tcp::ConnCenter::Create();
    if (NULL == conn_center)
    {
        LOG_ERROR("failed to alloc memory");
        return NULL;
    }

    int ret = -1;

    do
    {
        if (conn_center->Initialize(ctx) != 0)
        {
            break;
        }

        if (conn_center->Activate() != 0)
        {
            break;
        }

        if (!tcp_conn_center_set_.insert(conn_center).second)
        {
            break;
        }

        ret = 0;

    } while (0);

    if (ret != 0)
    {
        tcp_conn_center_set_.erase(conn_center);
        SAFE_DESTROY(conn_center);
        return NULL;
    }

    return conn_center;
}

http::ConnCenterInterface* ConnCenterMgr::CreateHttpConnCenter(const http::ConnCenterCtx* ctx)
{
    http::ConnCenter* conn_center = http::ConnCenter::Create();
    if (NULL == conn_center)
    {
        LOG_ERROR("failed to alloc memory");
        return NULL;
    }

    int ret = -1;

    do
    {
        if (conn_center->Initialize(ctx) != 0)
        {
            break;
        }

        if (conn_center->Activate() != 0)
        {
            break;
        }

        if (!http_conn_center_set_.insert(conn_center).second)
        {
            break;
        }

        ret = 0;

    } while (0);

    if (ret != 0)
    {
        http_conn_center_set_.erase(conn_center);
        SAFE_DESTROY(conn_center);
        return NULL;
    }

    return conn_center;
}

udp::ConnCenterInterface* ConnCenterMgr::CreateUdpConnCenter(const udp::ConnCenterCtx* ctx)
{
    udp::ConnCenter* conn_center = udp::ConnCenter::Create();
    if (NULL == conn_center)
    {
        LOG_ERROR("failed to alloc memory");
        return NULL;
    }

    int ret = -1;

    do
    {
        if (conn_center->Initialize(ctx) != 0)
        {
            break;
        }

        if (conn_center->Activate() != 0)
        {
            break;
        }

        if (!udp_conn_center_set_.insert(conn_center).second)
        {
            break;
        }

        ret = 0;

    } while (0);

    if (ret != 0)
    {
        udp_conn_center_set_.erase(conn_center);
        SAFE_DESTROY(conn_center);
        return NULL;
    }

    return conn_center;
}
}
