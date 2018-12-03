#ifndef CLIENT_CENTER_MGR_SRC_UDP_CLIENT_CENTER_H_
#define CLIENT_CENTER_MGR_SRC_UDP_CLIENT_CENTER_H_

#include "mem_util.h"
#include "udp_client.h"

namespace udp
{
class ClientCenter : public ModuleInterface, public ClientCenterInterface
{
    CREATE_FUNC(ClientCenter);

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

#endif // CLIENT_CENTER_MGR_SRC_UDP_CLIENT_CENTER_H_
