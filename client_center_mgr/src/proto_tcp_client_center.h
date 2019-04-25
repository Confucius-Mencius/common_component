#ifndef CLIENT_CENTER_MGR_SRC_PROTO_TCP_CLIENT_CENTER_H_
#define CLIENT_CENTER_MGR_SRC_PROTO_TCP_CLIENT_CENTER_H_

#include <map>
#include "proto_tcp_client.h"

namespace tcp
{
namespace proto
{
class ClientCenter : public ModuleInterface, public ClientCenterInterface
{
    CREATE_FUNC(ClientCenter)

public:
    ClientCenter();
    virtual ~ClientCenter();

    ///////////////////////// ModuleInterface /////////////////////////
    const char* GetVersion() const override;
    const char* GetLastErrMsg() const override;
    void Release() override;
    int Initialize(const void* ctx) override;
    void Finalize() override;
    int Activate() override;
    void Freeze() override;

    ///////////////////////// ClientCenterInterface /////////////////////////
    ClientInterface* CreateClient(const Peer& peer) override;
    void RemoveClient(const Peer& peer) override;
    ClientInterface* GetClient(const Peer& peer) override;

private:
    ClientCenterCtx client_center_ctx_;

    typedef std::map<Peer, Client*> ClientMap;
    ClientMap client_map_;
};
}
}

#endif // CLIENT_CENTER_MGR_SRC_PROTO_TCP_CLIENT_CENTER_H_
