#include "client_center_mgr.h"
#include "container_util.h"
#include "log_util.h"
#include "version.h"

namespace client_center_mgr
{
ClientCenterMgr::ClientCenterMgr() : tcp_client_center_set_(), http_client_center_set_(), udp_client_center_set_()
{
}

ClientCenterMgr::~ClientCenterMgr()
{
}

const char* ClientCenterMgr::GetVersion() const
{
    return CLIENT_CENTER_MGR_CLIENT_CENTER_MGR_VERSION;
}

const char* ClientCenterMgr::GetLastErrMsg() const
{
    return NULL;
}

void ClientCenterMgr::Release()
{
    RELEASE_CONTAINER(tcp_client_center_set_);
    RELEASE_CONTAINER(http_client_center_set_);
    RELEASE_CONTAINER(udp_client_center_set_);

    delete this;
}

int ClientCenterMgr::Initialize(const void* ctx)
{
//    if (NULL == ctx)
//    {
//        return -1;
//    }

//    client_center_mgr_ctx_ = *((const ClientCenterMgrCtx*) ctx);
    return 0;
}

void ClientCenterMgr::Finalize()
{
    FINALIZE_CONTAINER(tcp_client_center_set_);
    FINALIZE_CONTAINER(http_client_center_set_);
    FINALIZE_CONTAINER(udp_client_center_set_);
}

int ClientCenterMgr::Activate()
{
    return 0;
}

void ClientCenterMgr::Freeze()
{
    FREEZE_CONTAINER(tcp_client_center_set_);
    FREEZE_CONTAINER(http_client_center_set_);
    FREEZE_CONTAINER(udp_client_center_set_);
}

tcp::ClientCenterInterface* ClientCenterMgr::CreateTcpClientCenter(const tcp::ClientCenterCtx* ctx)
{
    tcp::ClientCenter* client_center = tcp::ClientCenter::Create();
    if (NULL == client_center)
    {
        LOG_ERROR("failed to alloc memory");
        return NULL;
    }

    int ret = -1;

    do
    {
        if (client_center->Initialize(ctx) != 0)
        {
            break;
        }

        if (client_center->Activate() != 0)
        {
            break;
        }

        if (!tcp_client_center_set_.insert(client_center).second)
        {
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        tcp_client_center_set_.erase(client_center);
        SAFE_DESTROY(client_center);
        return NULL;
    }

    return client_center;
}

http::ClientCenterInterface* ClientCenterMgr::CreateHttpClientCenter(const http::ClientCenterCtx* ctx)
{
    http::ClientCenter* client_center = http::ClientCenter::Create();
    if (NULL == client_center)
    {
        LOG_ERROR("failed to alloc memory");
        return NULL;
    }

    int ret = -1;

    do
    {
        if (client_center->Initialize(ctx) != 0)
        {
            break;
        }

        if (client_center->Activate() != 0)
        {
            break;
        }

        if (!http_client_center_set_.insert(client_center).second)
        {
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        http_client_center_set_.erase(client_center);
        SAFE_DESTROY(client_center);
        return NULL;
    }

    return client_center;
}

udp::ClientCenterInterface* ClientCenterMgr::CreateUdpClientCenter(const udp::ClientCenterCtx* ctx)
{
    udp::ClientCenter* client_center = udp::ClientCenter::Create();
    if (NULL == client_center)
    {
        LOG_ERROR("failed to alloc memory");
        return NULL;
    }

    int ret = -1;

    do
    {
        if (client_center->Initialize(ctx) != 0)
        {
            break;
        }

        if (client_center->Activate() != 0)
        {
            break;
        }

        if (!udp_client_center_set_.insert(client_center).second)
        {
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        udp_client_center_set_.erase(client_center);
        SAFE_DESTROY(client_center);
        return NULL;
    }

    return client_center;
}
}
