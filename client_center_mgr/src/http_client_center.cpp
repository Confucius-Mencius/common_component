#include "http_client_center.h"
#include "container_util.h"
#include "log_util.h"

namespace http
{
ClientCenter::ClientCenter() : client_center_ctx_(), client_map_()
{
}

ClientCenter::~ClientCenter()
{
}

const char* ClientCenter::GetVersion() const
{
    return nullptr;
}

const char* ClientCenter::GetLastErrMsg() const
{
    return nullptr;
}

void ClientCenter::Release()
{
    RELEASE_KV_CONTAINER(client_map_);
    delete this;
}

int ClientCenter::Initialize(const void* ctx)
{
    if (NULL == ctx)
    {
        return -1;
    }

    client_center_ctx_ = *((ClientCenterCtx*) ctx);
    return 0;
}

void ClientCenter::Finalize()
{
    FINALIZE_KV_CONTAINER(client_map_);
}

int ClientCenter::Activate()
{
    return 0;
}

void ClientCenter::Freeze()
{
    FREEZE_KV_CONTAINER(client_map_);
}

ClientInterface* ClientCenter::CreateClient(const Peer& peer)
{
    Client* client = Client::Create();
    if (NULL == client)
    {
        const int err = errno;
        LOG_ERROR("failed to create client, errno: " << err << ", err msg: " << strerror(err));
        return NULL;
    }

//    client->SetClientCenter(this);
    client->SetPeer(peer);

    int ret = -1;

    do
    {
        if (client->Initialize(&client_center_ctx_) != 0)
        {
            break;
        }

        if (client->Activate() != 0)
        {
            break;
        }

        if (!client_map_.insert(ClientMap::value_type(peer, client)).second)
        {
            break;
        }

        ret = 0;
    } while (0);

    if (ret != 0)
    {
        client_map_.erase(peer);
        SAFE_DESTROY(client);
        return NULL;
    }

    return client;
}

void ClientCenter::RemoveClient(const Peer& peer)
{
    ClientMap::iterator it = client_map_.find(peer);
    if (it != client_map_.end())
    {
        SAFE_DESTROY(it->second);
        client_map_.erase(it);
    }
}

ClientInterface* ClientCenter::GetClient(const Peer& peer)
{
    ClientMap::const_iterator it = client_map_.find(peer);
    if (it != client_map_.end())
    {
        return it->second;
    }

    return NULL;
}
}
